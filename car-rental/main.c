#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_CARS 12
#define NUM_CUSTOMERS 100
#define MAX_RENT_TIME 5
#define NUM_PREFERRED_CARS 3

sem_t available_cars;
int car_availability[NUM_CARS]; // Array to track the availability status of each car

typedef struct {
    int customer_id;
    int preferred_car_id;
    int rented_car_id;
} Customer;

void* rent_car(void* arg) {
    Customer *customer = (Customer*) arg;

    // Wait until a car is available and then rent it
    sem_wait(&available_cars);

    int rented_car_id = -1;

    int customerMood = rand()%2;
    int prefcar = customer->preferred_car_id;
    int prefcar_available = car_availability[prefcar];

    if(prefcar_available==1){
        rented_car_id=customer->preferred_car_id;
        car_availability[customer->preferred_car_id]=0;
    } else if(prefcar==-1 || customerMood<1){
        for (int i = 0; i < NUM_CARS; i++) {
            if (car_availability[i] == 1) { // Check if the car is available
                rented_car_id = i;
                car_availability[i] = 0; // Mark the car as unavailable
                break;
            }
        }
    } else {
        printf("Customer %d has rejected the available cars. He wanted car: %d\n", customer->customer_id, customer->preferred_car_id);

    }

    if (rented_car_id != -1) {
        customer->rented_car_id = rented_car_id;
        printf("Customer %d with preference %d is renting car %d\n", customer->customer_id, customer->preferred_car_id, customer->rented_car_id);

        // The car is used for some time
        sleep(rand() % MAX_RENT_TIME + 1);

        printf("Customer %d has returned car %d\n", customer->customer_id, customer->rented_car_id);

        // Mark the car as available
        car_availability[rented_car_id] = 1;

        // The car is available again
        sem_post(&available_cars);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t customers[NUM_CUSTOMERS];
    Customer customer_data[NUM_CUSTOMERS];

    // Initialize the semaphore with the number of available cars
    sem_init(&available_cars, 0, NUM_CARS);

    // Initialize car availability status (all cars are initially available)
    for (int i = 0; i < NUM_CARS; i++) {
        car_availability[i] = 1;
    }

    // Create threads for customers
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        customer_data[i].customer_id = i;
        int rand_num = rand();
        customer_data[i].preferred_car_id = rand_num % 2 == 0 ? -1 : rand_num % NUM_PREFERRED_CARS; // 50% chance of no preference
        pthread_create(&customers[i], NULL, rent_car, &customer_data[i]);
    }

    // Wait until all customers have returned their cars
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }
    printf("Every customer has been served.\n");

    sem_destroy(&available_cars);

    return 0;
}