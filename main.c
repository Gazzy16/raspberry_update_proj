#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>

typedef struct{ //структура в която се запазва информацията за всеки минерален блок в играта
	int minerals;
	pthread_mutex_t locked;
}Block;

typedef struct{ //структура със всички нужни за програмата променливи
	Block* blocks;
	int block_count;
	int scv_count;
	pthread_t scv_threads[200];
	int soldier_count;
	pthread_mutex_t cc_lock;
	int minerals_in_cc;
	pthread_t controls_thread;
}Game;

typedef struct{ //допълнителна структура в която се записва ID-то на всяко SCV
	int id;
	Game* game;
}SCV_args;

void SCV(void data){ //thread функция за SCV-та
	SCV_args *args = data;
	int id = args->id + 1;
	Game *game = args->game;
	free(args);
	int block = 0;
	while(1){ //безкраен цикъл в който се проверяват блоковете в играта и техните минерали, после дали съответния блок е свободен и тогава SCV-то започва да копае
			int i;
			for(i = 0; i < game->block_count; i++){ //цикъл в който се проверява дали в даден блок има минерали
				int mb = (i + block) % game->block_count;
				pthread_mutex_lock(&game->blocks[mb].locked);
				if(game->blocks[mb].minerals > 0){
					block = mb;
					pthread_mutex_unlock(&game->blocks[mb].locked);
					break;
				}
				pthread_mutex_unlock(&game->blocks[mb].locked);
			}
		
			if(i == game->block_count){
				break;
		}
		sleep(3);
		if(pthread_mutex_trylock(&game->blocks[block].locked) == 0){ //използвам pthread_mutex_trylock функцията за да проверя дали даден блок е заключен от SCV
			if(game->blocks[block].minerals > 0){ // проверявам дали след предишното SCV са останали минерали в блока
				printf("SCV %d is mining from mineral block %d\n", id, block+1);
				int minerals;
				if(game->blocks[block].minerals >= 8){  //ако минералите са повече или равни на 8 се изваждат от броя минерали в блока
					minerals = 8;
					game->blocks[block].minerals -= 8;
				}else{ 							//ако са по малко от 8 се изваждат толкова колкто са останали
					minerals = game->blocks[block].minerals;
					game->blocks[block].minerals = 0;
				}
				pthread_mutex_unlock(&game->blocks[block].locked); //след като е проверено че в блока има > от 0 минерала се изкопават
				printf("SCV %d is transporting minerals\n", id); //транспортирането на минерали
				sleep(2);
				pthread_mutex_lock(&game->cc_lock);
				game->minerals_in_cc += minerals;
				printf("SCV %d delivered minerals to the Command center\n", id); // складирането на минерлите в Command Center-а
				pthread_mutex_unlock(&game->cc_lock);
			}else{										//ако минералите в блока са < от 0 mutex-a се unlock-ва
				pthread_mutex_unlock(&game->blocks[block].locked);
			}
		}else{ //ако даден блок е зает в момента SCV-то преминава към следващия
			block++; 
			if (block == game->block_count){ //когато свърши броят блокове започва наново и опитва от първия блок
				block = 0;
			}
		}
		bool all_empty = true;
		for(int j = 0; j < game->block_count; j++){
			if(game->blocks[j].minerals > 0){
				all_empty = false;
			}
		}
		if(all_empty){
			break;
		}
	}

	return 0;
}

int main(int argc, char** argv){
	Game game;
	game.minerals_in_cc = 0;
	pthread_mutex_init(&game.cc_lock, NULL);
	game.soldier_count = 0;
	if(argc == 2){ //прави се проверка за аргументи подадени при стартирането на програмата
		game.block_count = atoi(argv[1]);
	}else{
		game.block_count = 2;
	}
	game.blocks = malloc(sizeof(Block)*game.block_count); //алокира се памет за блоковете и минералите им
	int i;
	for(i = 0; i < game.block_count; i++){ //задава се всеки блок да има по 500 минерала
		game.blocks[i].minerals = 500;
		pthread_mutex_init(&game.blocks[i].locked, NULL);
	}
	game.scv_count = 5;
	for(i = 0; i < game.scv_count; i++){
		SCV_args* args = malloc(sizeof(SCV_args));
		args->id = i;
		args->game = &game;
		int create_err = pthread_create(&game.scv_threads[i], NULL, SCV, args);
		if(create_err != 0){
			perror("Error while creating threads:");
		}
	}
	
	char l;
	while(game.soldier_count < 20){ //цикъл който работи докато не се създадат 20 войника
		scanf("%c", &l);
		
		switch(l){
			case 'm': {
				pthread_mutex_lock(&game.cc_lock);
				if(game.minerals_in_cc >= 50){
					game.minerals_in_cc -= 50;
					sleep(1);
					printf("You wanna piece of me, boy?\n");
					game.soldier_count++;
				}else{
					printf("Not enough mineral.\n");
				}
				pthread_mutex_unlock(&game.cc_lock);
				
				break;
			}
			case 's': {
				pthread_mutex_lock(&game.cc_lock);
				if(game.minerals_in_cc >= 50){
					game.minerals_in_cc -= 50;
					pthread_mutex_unlock(&game.cc_lock);
					SCV_args* args = malloc(sizeof(SCV_args));
					args->id = game.scv_count;
					args->game = &game;
					sleep(4);
					printf("SCV good to go, sir.\n");
					game.scv_count++;
					int create_err = pthread_create(&game.scv_threads[args->id], NULL, SCV, args);
					if(create_err != 0){
						perror("Error while creating threads:");
					}

				}else{
					printf("Not enough mineral.\n");
				}
				pthread_mutex_unlock(&game.cc_lock);
				
				break;
			}
		}
	}
	
	
	for(i = 0; i < game.scv_count; i++){
		int join_err = pthread_join(game.scv_threads[i], NULL);
		if(join_err != 0){
			perror("Error while joining threads:");
		}
	}
	
	pthread_mutex_destroy(&game.cc_lock);
	for(i = 0; i < game.block_count; i++){
		pthread_mutex_destroy(&game.blocks[i].locked);
	}
	free(game.blocks);
	printf("Map minerals %d, player minerals %d, SCVs %d, Marines %d\n", game.block_count*500, game.minerals_in_cc, game.scv_count, game.soldier_count);
	return 0;
}