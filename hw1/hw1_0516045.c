#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <sqlite3.h>
#define TRUE   1
#define FALSE  0

int result;

struct Login_User
{
	char username[256];
}login_user[30];

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   result = argc;
   // for(i=0; i<argc; i++){
   //    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   // }
   // printf("\n");
   return 0;
}

int main(int argc, char **argv)
{
	sqlite3 *db;
	int rc;
	rc = sqlite3_open("hw1", &db);
	// if( rc )
 //    {
 //    	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	// 	exit(0);
	// }
	// else
	// {
	// 	fprintf(stdout, "Opened database successfully\n");
	// }
	char *sql;
	char *zErrMsg = 0;
	sql = "CREATE TABLE USER(USERNAME TEXT, EMAIL TEXT, PASSWORD TEXT);";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   	// if( rc != SQLITE_OK )
   	// {
   	//   fprintf(stderr, "SQL error: %s\n", zErrMsg);
    //   sqlite3_free(zErrMsg);
    // }
    // else
    // {
    // 	fprintf(stdout, "Table created successfully\n");
   	// }
   	//sql = "CREATE TABLE LOGIN(USERNAME TEXT);";
    //rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    //if( rc != SQLITE_OK )
   	//{
   	//   fprintf(stderr, "SQL error: %s\n", zErrMsg);
    //   sqlite3_free(zErrMsg);
    // }
    // else
    // {
    // 	fprintf(stdout, "Table created successfully\n");
   	// }


    int port_number = atoi(argv[1]);
	int opt = TRUE;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30 ,activity, valread, sd;
    int max_sd;
    struct sockaddr_in info;


    char buffer[1025];
	fd_set readfds;
	char *welcome_message = "********************************\n" \
							"** Welcome to the BBS server. **\n" \
							"********************************\n" \
							"% ";
	
	for (int i=0;i<max_clients;i++)
	{
		client_socket[i] = 0;
	}
	if((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	bzero(&info,sizeof(info));
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_port = htons(port_number);

	if(bind(master_socket, (struct sockaddr *)&info, sizeof(info)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if(listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	addrlen = sizeof(info);

	for(int i=0;i<30;i++)
	{
		memset(login_user[i].username, '\0', 256);
	}
	while(TRUE)
	{
		FD_ZERO(&readfds);
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
		for (int i=0;i<max_clients;i++)
		{
			//socket descriptor  
			sd = client_socket[i];

			//if valid socket descriptor then add to read list  
			if(sd > 0)
				FD_SET( sd , &readfds);

			//highest file descriptor number, need it for the select function  
			if(sd > max_sd)
				max_sd = sd;
		}
		//wait for an activity on one of the sockets , timeout is NULL ,  
		//so wait indefinitely  
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

		if((activity < 0) && (errno!=EINTR))
		{
			printf("select error");
		}

		//If something happened on the master socket ,  
		//then its an incoming connection  
		if(FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket, (struct sockaddr *)&info, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			//inform user of socket number - used in send and receive commands  
			printf("New connection.\n");

			//send new connection greeting message  
			if(send(new_socket, welcome_message, strlen(welcome_message), 0) != strlen(welcome_message))
			{
				perror("send");
			}

			

			for (int i = 0; i < max_clients; i++)
			{
				if(client_socket[i] == 0)
				{
					client_socket[i] = new_socket;
					
					break;
				}
			}
		}
		int socket_index;
		for(socket_index=0;socket_index<max_clients;socket_index++)
		{
			result = 0;
			sd = client_socket[socket_index];
			if(FD_ISSET(sd , &readfds))
			{
				if((valread = read(sd, buffer, 1024)) == 0)
				{
					printf("%d disconnect\n", socket_index);
					close(sd);
					client_socket[socket_index] = 0;
					memset(login_user[socket_index].username, '\0', 256);
				}
				else
				{
					buffer[valread] = '\0';
					if(buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'i' && buffer[3] == 't')
					{
						close(sd);
						client_socket[socket_index] = 0;
						memset(login_user[socket_index].username, '\0', 256);
					}
					else if(buffer[0] == 'r' && buffer[1] == 'e' && buffer[2] == 'g' && buffer[3] == 'i' && buffer[4] == 's' && buffer[5] == 't' && buffer[6] == 'e' && buffer[7] == 'r')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 3)
						{
							char *message = "Usage: register <username> <email> <password>\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							char username[256], email[256], password[256];
							int i, j, k, index=0;
							for(i=9;i<strlen(buffer);i++)
							{
								if(buffer[i] == ' ')
								{
									break;
								}
								username[index] = buffer[i];
								index++;
							}
							username[index] = '\0';
							index = 0;
							for(j=i+1;j<strlen(buffer);j++)
							{
								if(buffer[j] == ' ')
								{
									break;
								}
								email[index] = buffer[j];
								index++;
							}
							email[index] = '\0';
							index = 0;
							for(k=j+1;k<strlen(buffer);k++)
							{
								if(buffer[k] == '\r' || buffer[k] == '\n')
								{
									break;
								}
								password[index] = buffer[k];
								index++;
							}
							password[index] = '\0';
							// printf("%s %s %s\n", username, password, email);
							char sql[256] = "";
							strcat(sql, "SELECT * ");
							strcat(sql, " from USER WHERE USERNAME='");
							strcat(sql, username);
							strcat(sql, "';");
							rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
							// if(rc != SQLITE_OK)
							// {
							// 	fprintf(stderr, "SQL error: %s\n", zErrMsg);
							// 	sqlite3_free(zErrMsg);
							// }
							// else
							// {
							// 	fprintf(stdout, "Operation done successfully\n");
							// }
							// printf("%d", result);
							if(result != 0)
							{
								char *message = "Username is already used.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char sql[256] = "";
								strcat(sql, "INSERT INTO USER (USERNAME, EMAIL, PASSWORD) VALUES ('");
								strcat(sql, username);
								strcat(sql, "', '");
								strcat(sql, email);
								strcat(sql, "', '");
								strcat(sql, password);
								strcat(sql, "'); ");
								rc = sqlite3_exec(db, sql, callback, "0", &zErrMsg);
								// if( rc != SQLITE_OK )
								// {
								// 	fprintf(stderr, "SQL error5: %s\n", zErrMsg);
								// 	sqlite3_free(zErrMsg);
								// }
								// else
								// {
								// 	fprintf(stdout, "Operation done successfully\n");
								// }
								char *message = "Register successfully.\n";
								send(sd, message, strlen(message), 0);
							}
						}
					}
					else if(buffer[0] == 'l' && buffer[1] == 'o' && buffer[2] == 'g' && buffer[3] == 'i' && buffer[4] == 'n')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 2)
						{
							char *message = "Usage: login <username> <password>\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							if(strlen(login_user[socket_index].username) != 0)
							{
								char *message = "Please logout first.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char username[256], password[256];
								int i, j, index=0;
								for(i=6;i<strlen(buffer);i++)
								{
									if(buffer[i] == ' ')
									{
										break;
									}
									username[index] = buffer[i];
									index++;
								}
								username[index] = '\0';
								index = 0;
								for(j=i+1;j<strlen(buffer);j++)
								{
									if(buffer[j] == '\r' || buffer[j] == '\n')
									{
										break;
									}
									password[index] = buffer[j];
									index++;
								}
								password[index] = '\0';
								char sql[256] = "";
								strcat(sql, "SELECT * ");
								strcat(sql, "from USER WHERE USERNAME='");
								strcat(sql, username);
								strcat(sql, "' AND PASSWORD='");
								strcat(sql, password);
								strcat(sql, "';");
								rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
								// if(rc != SQLITE_OK)
								// {
								// 	fprintf(stderr, "SQL error: %s\n", zErrMsg);
								// 	sqlite3_free(zErrMsg);
								// }
								// else
								// {
								// 	fprintf(stdout, "Operation done successfully\n");
								// }
								if(result == 0)
								{
									char *message = "Login failed.\n";
									send(sd, message, strlen(message), 0);
								}
								else
								{
									strcpy(login_user[socket_index].username, username);
									char message[256] = "";
									strcat(message, "Welcome, ");
									strcat(message, username);
									strcat(message, ".\n");
									send(sd, message, strlen(message), 0);
								}
							}
						}
					}
					else if(buffer[0] == 'l' && buffer[1] == 'o' && buffer[2] == 'g' && buffer[3] == 'o' && buffer[4] == 'u' && buffer[5] == 't')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 0)
						{
							char *message = "Usage: logout\n";
							send(sd, message, strlen(message), 0);
						}
						if(strlen(login_user[socket_index].username) == 0)
						{
							char *message = "Please login first.\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							char *message = "Bye, ";
							send(sd, message, strlen(message), 0);
							message = login_user[socket_index].username;
							send(sd, message, strlen(message), 0);
							message = ".\n";
							send(sd, message, strlen(message), 0);
							memset(login_user[socket_index].username, '\0', 256);
						}
					}
					else if(buffer[0] == 'w' && buffer[1] == 'h' && buffer[2] == 'o' && buffer[3] == 'a' && buffer[4] == 'm' && buffer[5] == 'i')
					{
						int count = 0;
						for(int i=0;i<strlen(buffer);i++)
						{
							if(buffer[i] == ' ')
							{
								count++;
							}
						}
						if(count != 0)
						{
							char *message = "Usage: whoami\n";
							send(sd, message, strlen(message), 0);
						}
						else
						{
							if(strlen(login_user[socket_index].username) == 0)
							{
								char *message = "Please login first.\n";
								send(sd, message, strlen(message), 0);
							}
							else
							{
								char *message = login_user[socket_index].username;
								send(sd, message, strlen(message), 0);
								message = "\n";
								send(sd, message, strlen(message), 0);
							}
						}
					}
					else
					{
						char *message = "Wrong command\n" \
										"Usage: register <username> <email> <password>\n" \
										"       login <username> <password>\n" \
										"       logout\n" \
										"       whoami\n" \
										"       exit\n";
						send(sd, message, strlen(message), 0);
					}
				}
				char *prompt = "% ";
				send(sd, prompt, strlen(prompt), 0);
			}
		}
	}
	sqlite3_close(db);

}