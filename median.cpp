#include "heaplib.h"

#include <boost/algorithm/string.hpp>

#include <arpa/inet.h>
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string>
#include <sys/socket.h>
#include <tuple>
#include <unistd.h> 
#include <vector>

class Client {
    private:
        int port;
        int sock_fd;
        struct sockaddr_in serv_addr;
    public:
        Client() { }
        
        Client(std::string host_ip, int port) {
            // Set up the client.
            this->port = port;

            if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
                perror("\n Socket creation error \n"); 
                exit(EXIT_FAILURE);
            }

            serv_addr.sin_family = AF_INET; 
            serv_addr.sin_port = htons(port);

            // Convert IPv4 and IPv6 addresses from text to binary form 
            if(inet_pton(AF_INET, host_ip.c_str(), &serv_addr.sin_addr)<=0)  
            { 
                printf("\nInvalid address/ Address not supported \n"); 
                exit(EXIT_FAILURE);
            }
        }

        int client_connect() {
            if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
                printf("\nConnection Failed \n"); 
                return -1; 
            }
            return 0;           
        }

        int client_query(char *buffer, int max_length) {
            char *req = "query";
            int retval = send(sock_fd, req, strlen(req), 0);
            int valread = read(sock_fd, buffer, max_length);
            return retval;
        }

        int client_split(char *buffer, int max_length, int pivot_val) {
            std::string req = "split|" + std::to_string(pivot_val);
            printf("Split message: %s\n", req.c_str());
            int retval = send(sock_fd, req.c_str(), strlen(req.c_str()), 0);
            int valread = read(sock_fd, buffer, max_length);
            return retval; 
        }

        int client_discard(char *buffer, int max_length, int discard_section) {
            std::string req = "discard|" + std::to_string(discard_section);
            int retval = send(sock_fd, req.c_str(), strlen(req.c_str()), 0);
            int valread = read(sock_fd, buffer, max_length);
            return retval;
        }

        int client_send(char *buffer, int max_length) {
            char *req = "median"; 
            int retval = send(sock_fd, req, strlen(req), 0);
            int valread = read(sock_fd, buffer, max_length);
            return retval;       
        }

        int get_port() {
            return port;
        }
};

class NumberList {
    private:
        std::vector<int> numbers;
        int min_val;
        int max_val;
        int last_index;

    public:
        NumberList() {
            //Generate random entries for the number list.
            int length = 101;
            for(int i = 0; i < length; i++) {
                numbers.push_back(rand() % 100);
            }
            heap_sort(&numbers);
            reset();
        }

        void reset() {
            min_val = 0;
            max_val = numbers.size()-1;
        }

        std::tuple<int, int> get_above_and_below(int pivot_val) {
            // Eh, just do a linear search for now and say fuck it.
            int i;
            for(i = min_val; i <= max_val; i++) {
                if(numbers[i] >= pivot_val) {
                    break;
                }
            }
            last_index = i;
            // pivot_val (or a number bigger than it) first appears at index i.
            // The number of ints below pivot_val are all the indices below it.
            // The number of ints above pivot_val are the other vals (numbers.size() - i)
            return std::make_tuple(i, numbers.size() - i);
        }

        // Pick a number from [min_val, max_val) and put it in ptr.
        // Returns 0 if successful, -1 if otherwise.
        int get_random_val(int *ptr) {
            if(max_val < min_val || min_val > max_val) {
                return -1;
            }
            int r = (rand() % (max_val - min_val + 1)) + min_val;
            printf("RANDOM VALUE FROM INDEX %d", r);
            *ptr = numbers[r];
            return 0;
        }

        void discard_section(int section) {
            if(section == 0) {
                min_val = std::max<int>(0, last_index + 1);
            }
            else if(section == 1) {
                max_val = std::min<int>(max_val, last_index - 1);
            }
            printf("\n");
            printf("Can only return values from index %d to index %d\n", min_val, max_val);
            printf("Can only return values from %d to %d\n", numbers[min_val], numbers[max_val]);
            printf("\n");
        }

        void print_list() {
            for(int i = 0; i < numbers.size(); i++)
                printf("%d\n", numbers[i]);
        }

        int get_list_length() {
            return numbers.size();
        }
};

class Server {
    private:
        int port;
        int server_fd;
        struct sockaddr_in address;
        NumberList nlist;

    public:
        Server(int port) {
            // General server setup. Do not start accepting connections.
            this->port = port;
            this->nlist = NumberList();

            // Create the socket file descriptor
            if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
                perror("socket failed"); 
                exit(EXIT_FAILURE); 
            } 

            // Unnecessary but can be helpful for dealing with various reuse errors.
            int opt = 1;
            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) { 
                perror("setsockopt"); 
                exit(EXIT_FAILURE); 
            } 

            address.sin_family = AF_INET; 
            address.sin_addr.s_addr = INADDR_ANY; 
            // Change byte order to network order (if machine's byte order is not already little endian)
            address.sin_port = htons( port );

            // Bind socket to the port 8080 
            if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
                perror("bind failed"); 
                exit(EXIT_FAILURE); 
            } 

            // Create a backlog that holds up to 3 pending connections.
            if (listen(server_fd, 3) < 0) { 
                perror("listen"); 
                exit(EXIT_FAILURE); 
            }

            nlist.print_list();
        }

        int process_one() {
            int new_socket;
            int valread;
            int addrlen = sizeof(address);
            char buffer[1024] = {0};

            // Handle a single connection.
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
                perror("accept"); 
                exit(EXIT_FAILURE); 
            }
            valread = read(new_socket, buffer, 1024);
            while(valread > 0) {
                int pivot_val = 0; 
                printf("recvd: %s\n", buffer);
                std::string command = buffer;
                if(strcmp(command.c_str(), "query") == 0) {
                    // Pick a random value from the numberlist and return it
                    // If there are no remaining numbers (min_vlaue == max_value), return "none"
                    printf("Inside query...\n");
                    int random_val = 0;
                    std::string resp;
                    if(nlist.get_random_val(&random_val) == -1) {
                        // No can do pardner
                        printf("\n\n\n\nWARNING\n\n\n\n");
                        resp = "none";
                    }
                    else {
                        // Random value is in random_val
                        resp = std::to_string(random_val);
                    }

                    printf("Response: %s\n", resp.c_str());
                    int retval = send(new_socket, resp.c_str(), strlen(resp.c_str()), 0);                
                }
                else if(strcmp(command.substr(0, 5).c_str(), "split") == 0) {
                    printf("Inside split...\n");
                    std::vector<std::string> result;
                    boost::split(result, command, boost::is_any_of("|"));
                    pivot_val = std::stoi(result[1]);
                    printf("PIVOT VAL: %d\n", pivot_val);
                    std::tuple<int, int> above_and_below = nlist.get_above_and_below(pivot_val);
                    printf("Below: %d\n", std::get<0>(above_and_below));
                    printf("Above: %d\n", std::get<1>(above_and_below));
                    std::string resp = std::to_string(nlist.get_list_length()) + "|" + std::to_string(std::get<0>(above_and_below)) + "|" + std::to_string(std::get<1>(above_and_below));
                    
                    printf("Response: %s\n", resp.c_str());

                    int retval = send(new_socket, resp.c_str(), strlen(resp.c_str()), 0);
                }
                else {
                    // Buffer should be of the form "discard|{0,1}", with 0 telling the machine to dump their lower section and 1 the higher
                    printf("Inside discard...\n");
                    std::vector<std::string> result;
                    std::string resp = "0";
                    boost::split(result, command, boost::is_any_of("|"));
                    int partition_to_drop = std::stoi(result[1]);
                    nlist.discard_section(partition_to_drop);
                    
                    printf("Response: %s\n", resp.c_str());

                    int retval = send(new_socket, resp.c_str(), strlen(resp.c_str()), 0);
                }
                memset(&buffer[0], 0, 1024);
                printf("Waiting for next message...\n");
                valread = recv(new_socket, buffer, 1024, 0);
                printf("Next message received!\n");
            }
            nlist.reset();
            printf("Reset.");
            return 0;
        }

        int get_port() {
            return port;
        }
};

int main(int argc, char const *argv[]) 
{ 
    setbuf(stdout, NULL);
    srand(time(nullptr));
    std::string slave_ips[3] = {
        "10.0.0.200",
        "10.0.0.4",
        "10.0.0.6"
        };
    int size = sizeof(slave_ips)/sizeof(slave_ips[0]);

    if(argc != 2) {
        printf("Usage: ./server {0,1}");
        printf("0 -> slave node");
        printf("1 -> master node");
    }
    if (atoi(argv[1]) == 0){
        printf("Slave mode active.\n");
        Server s = Server(8080);

        while(1)
            s.process_one();
    }
    else {
        // Create n clients (one for each slave node)
        printf("Master mode active.\n");
        int sum = 0;
        int count = 0;
        Client clients[3];
        for(int i = 0; i < size; i++) {
            clients[i] = Client(slave_ips[i], 8080);
            clients[i].client_connect();
        }

        int found = 0;
        int size =  (sizeof(clients)/sizeof(clients[0]));
        printf("SIZE: %d\n", size);
        while(!found) {
            
            char buffer[1024] = {0};
            int server_to_query = (rand() % size);
            Client c = clients[server_to_query];
            printf("Sleeping before next query...\n");
            sleep(2);
            c.client_query(buffer, 1024);
            printf("Sent out query...\n");
            while(strcmp(buffer, "none") == 0) {
                sleep(1);
                memset(&buffer, 0, 1024);
                server_to_query = (rand() % size);
                c = clients[server_to_query];
                c.client_query(buffer, 1024);
                printf("Sent out another query...\n");
            } 

            // std::vector<std::string> result;
            // boost::split(result, buffer, boost::is_any_of("|"));

            printf("%s\n", buffer);

            int pivot_val = std::stoi(buffer);
            
            // Note that above starts at -1 because the pivot is counted as above, even though it shouldn't be included in the count.
            int below = 0;
            int above = -1;
            for(int i = 0; i < size; i++) {
                char buffer2[1024] = {0};
                printf("Splitting on pivot val %d\n", pivot_val);
                clients[i].client_split(buffer2, 1024, pivot_val);
                printf("%s\n", buffer2);
                std::vector<std::string> result;
                boost::split(result, buffer2, boost::is_any_of("|"));
                below += std::stoi(result[1]);
                above += std::stoi(result[2]);
            }
            if(below > above or above > below) {
                int discard_section = (below > above)? 1 : 0;
                for(int i = 0; i < size; i++) {
                    char buffer2[1024] = {0};
                    printf("Discarding section %d\n", discard_section);
                    clients[i].client_discard(buffer2, 1024, discard_section);
                    printf("%s\n", buffer2);
                }    
            }
            else {
                found = 1;
                printf("Median found! %d", pivot_val);
            }
        }

    }
    return 0; 
} 