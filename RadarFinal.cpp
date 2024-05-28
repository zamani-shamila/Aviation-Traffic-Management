#include <unordered_map>
#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <thread>
#include<iostream>
#include <vector>
#include <sys/dispatch.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <random>
using namespace std;

#define Interval	30
#define MinSpeed	70	//typical min speed for aircraft is 700 ft/s
#define MaxSpeed	100	//typical max speed for aircraft is 1000 ft/s
#define NumberOfAircraft	40	//Number of Aircrafts
//char bffer[256];
//bool flag = 0;
long long elapsed_time_temp;


/* Aircraft Class with two Costructor
one for initializing id and oter one for initializing the aircraft's ID, position, and speed.
*/
class Aircraft {
public:
	Aircraft() :
			ID(nextID) {
//		ID = nextID;
//		setID(nextID);
	}

	Aircraft(int ID, int x, int y, int z, int speed_x, int speed_y, int speed_z) :
			ID(nextID++), x(x), y(y), z(z), speed_x(speed_x), speed_y(speed_y), speed_z(
					speed_z) {
	}

	void setID(int newID) {
		ID = newID;
	}

	int getID() {
		return ID;
	}

	void move() {
		x += speed_x;
		y += speed_y;
		z += speed_z;
	}

	int ID, x, y, z, speed_x, speed_y, speed_z;
	static int nextID;
	static std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

	void print() {
		std::chrono::steady_clock::time_point now =
				std::chrono::steady_clock::now();
		long long elapsed_time = std::chrono::duration_cast<
				std::chrono::milliseconds>(now - start_time).count();
		elapsed_time_temp=elapsed_time;
		std::cout << "Time: " << elapsed_time << " ms, Aircraft ID: " << ID
				<< " Position: (" << x << ", " << y << ", " << z << ") Speed: ("
				<< speed_x << ", " << speed_y << ", " << speed_z << ")\n";
	}

};

int Aircraft::nextID = 1;
std::chrono::time_point<std::chrono::high_resolution_clock> Aircraft::start_time;

/* Aircraft Class with two Costructor
one for setting id and oter one for setting all the required things
*/

class Airspace {
public:

	int top_altitude;
	int bottom_altitude;
	int xmin;
	int xmax;
	int ymin;
	int ymax;


	Airspace(int width, int length, int height) :
	    width{width},
	    length{length},
	    height{height},
	    top_altitude{height},
	    bottom_altitude{height - 15000},
	    xmin{-width / 2},
	    xmax{width / 2},
	    ymin{-length / 2},
	    ymax{length / 2}
	  {}
	void addAircraft(int ID, int x, int y, int z, int speed_x, int speed_y,
			int speed_z) {
		aircrafts[ID] = Aircraft(ID, x, y, z, speed_x, speed_y, speed_z);
	}
	Aircraft& getAircraft(int ID) {
		return aircrafts[ID];
	}

	void moveAircrafts() {
		for (auto &pair : aircrafts) {
			pair.second.move();
		}
	}

	int width, length, height;
	std::unordered_map<int, Aircraft> aircrafts;

};

//pthread_attr_t attr;
// Returns a random x-coordinate between 0 and 100000

int randomXPosition() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100000);
    return dis(gen);
}

// Returns a random y-coordinate between 0 and 100000
int randomYPosition() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100000);
    return dis(gen);
}

// Returns a random z-coordinate between 15000 and 25000
int randomZPosition() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(15000, 25000);
    return dis(gen);
}
/*
 * Airspace class shows a 3D airspace. It has a constructor for initializing the dimensions of the airspace.
 * The class has methods to add an aircraft to the airspace, get an aircraft by its ID,
 * and move all aircraft within the airspace.
 * */
void* createAircraft(void *arg) {
	Airspace *airspace = static_cast<Airspace*>(arg);
	int x = randomXPosition();
	int y = randomYPosition();
	int z = randomZPosition();

	// Ensure that the plane is within the new airspace
	while (x > 100000 || y > 100000 || z > 25000) {
		x = randomXPosition();
		y = randomYPosition();
		z = randomZPosition();
	}

	int speed_x = rand() % Interval + MinSpeed;
	int speed_y = rand() % Interval + MinSpeed;
	int speed_z = rand() % Interval + MinSpeed;

	airspace->aircrafts[Aircraft::nextID] = Aircraft(Aircraft::nextID, x, y, z,
			speed_x, speed_y, speed_z);

	pthread_exit(NULL);
}

void* OpConsole(void *arg) {
	std::ofstream Commandlogfile;
	Airspace *airspace = (Airspace*) arg;
	struct timespec sleep_time;
	sleep_time.tv_sec = 1;
	sleep_time.tv_nsec = 0;

	Commandlogfile.open("log_history.txt", std::ios_base::app);
	while (true) {
		airspace->moveAircrafts();

		for (auto &pair : airspace->aircrafts) {
			pair.second.print();
		}

		///////////// Violation /////////////////
		std::set<std::pair<int, int>> checked_pairs;
		for (auto &pair1 : airspace->aircrafts) {
			for (auto &pair2 : airspace->aircrafts) {

				if (&pair1.second != &pair2.second) { // Avoid comparing an aircraft with itself
					if (pair1.first < pair2.first) {
						int distance_x = abs(pair1.second.x - pair2.second.x);
						int distance_y = abs(pair1.second.y - pair2.second.y);
						int distance_z = abs(pair1.second.z - pair2.second.z);
						int speed_x_diff = abs(
								pair1.second.speed_x - pair2.second.speed_x);
						int speed_y_diff = abs(
								pair1.second.speed_y - pair2.second.speed_y);
						int speed_z_diff = abs(
								pair1.second.speed_z - pair2.second.speed_z);
						double time_to_collision = (distance_x * 1.0
								/ speed_x_diff + distance_y * 1.0 / speed_y_diff
								+ distance_z * 1.0 / speed_z_diff) * 1000;
						if (time_to_collision < 180000 && distance_x < 3000
								&& distance_y < 3000 && distance_z < 1000) {
							std::pair<int, int> aircraft_pair = std::make_pair(
									pair1.second.getID(), pair2.second.getID());

// Only print the violation if the pair has not been checked before

							std::cout
									<< "Safety violation detected between Aircraft "
									<< pair1.second.getID() << " and Aircraft "
									<< pair2.second.getID()
									<< " in the next 3 minutes." << std::endl;
/////////////input////////////

							std::cout
									<< "ALERT: Possible collision between Aircraft "
									<< pair1.second.getID() << " and Aircraft "
									<< pair2.second.getID() << " detected."
									<< std::endl;

							std::cout
									<< "Please enter new speeds and positions for the aircraft."
									<< std::endl;

							std::cout << "Aircraft " << pair1.second.getID()
									<< " (Current position: (" << pair1.second.x
									<< ", " << pair1.second.y << ", "
									<< pair1.second.z << "), Current speed: ("
									<< pair1.second.speed_x << ", "
									<< pair1.second.speed_y << ", "
									<< pair1.second.speed_z << ")): ";
							int new_speed_x1, new_speed_y1, new_speed_z1,
									new_pos_x1, new_pos_y1, new_pos_z1;
							std::cin >> new_pos_x1 >> new_pos_y1 >> new_pos_z1
									>> new_speed_x1 >> new_speed_y1
									>> new_speed_z1;

							std::cout << "Aircraft " << pair2.second.getID()
									<< " (Current position: (" << pair2.second.x
									<< ", " << pair2.second.y << ", "
									<< pair2.second.z << "), Current speed: ("
									<< pair2.second.speed_x << ", "
									<< pair2.second.speed_y << ", "
									<< pair2.second.speed_z << ")): ";
							int new_speed_x2, new_speed_y2, new_speed_z2,
									new_pos_x2, new_pos_y2, new_pos_z2;
							std::cin >> new_pos_x2 >> new_pos_y2 >> new_pos_z2
									>> new_speed_x2 >> new_speed_y2
									>> new_speed_z2;

// Update the speeds and positions of the aircrafts
							pair1.second.speed_x = new_speed_x1;
							pair1.second.speed_y = new_speed_y1;
							pair1.second.speed_z = new_speed_z1;
							pair1.second.x = new_pos_x1;
							pair1.second.y = new_pos_y1;
							pair1.second.z = new_pos_z1;
							pair2.second.speed_x = new_speed_x2;
							pair2.second.speed_y = new_speed_y2;
							pair2.second.speed_z = new_speed_z2;
							pair2.second.x = new_pos_x2;
							pair2.second.y = new_pos_y2;
							pair2.second.z = new_pos_z2;


							std::cout
									<< "New speeds and positions for Aircraft "
									<< pair1.second.getID() << " and Aircraft "
									<< pair2.second.getID() << " set.\n"
									<< std::endl;

//////////Command Log//////////

							Commandlogfile
									<< "New speeds and positions for Aircraft "
									<< pair1.second.getID() << " and Aircraft "
									<< pair2.second.getID() << " set.\n"
									<< "Aircraft " << pair1.second.getID()
									<< " (Current position (" << pair1.second.x
									<< ", " << pair1.second.y << ", "
									<< pair1.second.z << "), Current speed: ("
									<< pair1.second.speed_x << ", "
									<< pair1.second.speed_y << ", "
									<< pair1.second.speed_z << ")) "
									<< "Aircraft " << pair2.second.getID()
									<< " (Current position (" << pair2.second.x
									<< ", " << pair2.second.y << ", "
									<< pair2.second.z << "), Current speed: ("
									<< pair2.second.speed_x << ", "
									<< pair2.second.speed_y << ", "
									<< pair2.second.speed_z << "))\n"
									<< std::endl;
							usleep(100000);

						}
					}
				}
			}
		}
/////////////////////////////////////////Aircraft Exit///
// Check if aircrafts are still within airspace boundaries

//		for (auto &pair : airspace->aircrafts) {
//			Aircraft &aircraft = pair.second;
//			if (aircraft.x < airspace->xmin || aircraft.x > airspace->xmax
//					|| aircraft.y < airspace->ymin
//					|| aircraft.y > airspace->ymax
//					|| aircraft.z < airspace->bottom_altitude
//					|| aircraft.z > airspace->top_altitude) {
//				// Aircraft has left airspace boundaries, remove it
//				aircrafts_to_remove.push_back(aircraft.getID());
//			} else {
//				aircraft.print();
//			}
//		}
//		for (int id : aircrafts_to_remove) {
//			airspace->aircrafts.erase(id);
//		}

/*Aircraft Exit of boundaries
 * Checking aircrafts are still within airspace boundaries
*/
		std::vector<int> aircrafts_to_remove;
		for (auto &pair : airspace->aircrafts) {
			Aircraft &aircraft = pair.second;
			if (aircraft.x < airspace->xmin || aircraft.x > airspace->xmax
					|| aircraft.y < airspace->ymin
					|| aircraft.y > airspace->ymax
					|| aircraft.z < airspace->bottom_altitude
					|| aircraft.z > airspace->top_altitude) {
				// Aircraft has left airspace boundaries, remove it and print message
				aircrafts_to_remove.push_back(aircraft.getID());
				std::cout << "The aircraft with ID " << aircraft.getID()
						<< " has exited the airspace boundaries." << std::endl;
//				sscanf(bffer,
//						"The aircraft with ID %d has exited the airspace boundaries.",
//						aircraft.getID());
//				flag = 1;
//				usleep(10000);
			} else {
				aircraft.print();
			}
		}
		for (int id : aircrafts_to_remove) {
			airspace->aircrafts.erase(id + 1);
		}

//		for (auto &pair : airspace->aircrafts) {
//			pair.second.print();
//		}

		nanosleep(&sleep_time, NULL);
	}

	return NULL;
}

//Write Airspace History to Log File
void* writeHistoryThread(void *arg) {
	Airspace *airspace = (Airspace*) arg;
	std::ofstream historyfile;
	historyfile.open("history.txt", std::ios_base::app);
	std::chrono::steady_clock::time_point now =
							std::chrono::steady_clock::now();
	while (true) {


		for (auto &pair : airspace->aircrafts) {
			historyfile << "Time: "<<elapsed_time_temp<<" ms Aircraft ID: " << pair.second.getID() << " Position: ("
					<< pair.second.x << ", " << pair.second.y << ", "
					<< pair.second.z << ") Speed: (" << pair.second.speed_x
					<< ", " << pair.second.speed_y << ", "
					<< pair.second.speed_z << ")\n";
		}

		std::this_thread::sleep_for(std::chrono::seconds(30));
//		usleep(30000000);
	}
	return NULL;
}

///////////////////CompSys Part/////////////////
//Display//Send Aircraft Information and Collision Alerts to Comp System

#define ATTACH_POINT "RadarToCompSys"

typedef struct _pulse msg_header_t;

typedef struct _my_data {
	msg_header_t hdr;
	char aircraftinfo[256]; // Modify this structure to include the data fields that will be sent by the radar
} my_data_t;

void* SendToCompSys(void *arg) {
	Airspace *airspace = (Airspace*) arg;
	my_data_t msg;
	struct timespec sleep_time2;
	sleep_time2.tv_sec = 1;
	sleep_time2.tv_nsec = 0;

	int server_coid; //server connection ID

	if ((server_coid = name_open(ATTACH_POINT, 0)) == -1) {
		printf("Error opening name: %s\n", strerror(errno));
	}
	/* We would have pre-defined data to stuff here */
	msg.hdr.type = 0x00;
	msg.hdr.subtype = 0x01;

	static std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
	std::chrono::steady_clock::time_point now =
			std::chrono::steady_clock::now();
	long long elapsed_time = std::chrono::duration_cast<
			std::chrono::milliseconds>(now - start_time).count();
	while (true) {

		for (auto &pair : airspace->aircrafts) {
			auto current_time = std::chrono::system_clock::now();
			std::time_t current_time_t = std::chrono::system_clock::to_time_t(
					current_time);
			sprintf(msg.aircraftinfo,
					"Plane ID:%d Position: (%d),(%d),(%d) Speed: (%d),(%d),(%d) ,%ld ms \n",
					pair.second.getID(), pair.second.x, pair.second.y,
					pair.second.z, pair.second.speed_x, pair.second.speed_y,
					pair.second.speed_z, elapsed_time);

//			if (flag == 1) {
//				flag = 0;
//				strcpy(msg.aircraftinfo, bffer);
//				MsgSend(server_coid, &msg, sizeof(msg), NULL, 0);
//			}

			if (MsgSend(server_coid, &msg, sizeof(msg), NULL, 0) == -1) {
				printf("Error sending message to server.\n");
			}

		}

		for (auto &pair : airspace->aircrafts) {

			for (auto &pair2 : airspace->aircrafts) {

				if (&pair.second != &pair2.second) { // Avoid comparing an Plane with itself
					int distance_x = abs(pair.second.x - pair2.second.x);
					int distance_y = abs(pair.second.y - pair2.second.y);
					int distance_z = abs(pair.second.z - pair2.second.z);
					int speed_x_diff = abs(
							pair.second.speed_x - pair2.second.speed_x);
					int speed_y_diff = abs(
							pair.second.speed_y - pair2.second.speed_y);
					int speed_z_diff = abs(
							pair.second.speed_z - pair2.second.speed_z);
					double time_to_collision = (distance_x * 1.0 / speed_x_diff
							+ distance_y * 1.0 / speed_y_diff
							+ distance_z * 1.0 / speed_z_diff) * 1000;

					if (time_to_collision < 180000 && distance_x < 3000
							&& distance_y < 3000 && distance_z < 1000) { // Check for collisions within the next 3 minutes and within specified distance

						sprintf(msg.aircraftinfo,
								"Safety violation detected between Plane %d and Plane %d in the next 3 minutes. ",
								pair.second.getID(), pair2.second.getID());
						if (MsgSend(server_coid, &msg, sizeof(msg), NULL, 0)
								== -1) {
							printf("Error sending message to server.\n");

						}
					}
				}
			}

		}

		usleep(5000000); ///// 5000000 micro second==5second
	}

	return NULL;
}

int main() {
	// Initialize Program and Airspace Parameters

	// Seed the random number generator with the current time
	srand(time(0));
	Airspace airspace(100000, 100000, 25000);

	// Create Aircraft Threads and Move Them Within Airspace
	for (int i = 0; i < NumberOfAircraft; ++i) {
		pthread_t thread;
		int rc = pthread_create(&thread, NULL, createAircraft, &airspace);
		if (rc) {
			printf("ERROR when creating aircraft thread; Code is %d\n", rc);
		}
		pthread_join(thread, NULL);
	}
// Operator Console for controlling aircrafts
	pthread_t OpConsole_thread;
	int rc = pthread_create(&OpConsole_thread, NULL, OpConsole, &airspace);
	if (rc) {
		printf("ERROR when creating OpConsole Code is %d\n", rc);
	}
	// Write Airspace History to Log File
	pthread_t history_thread;
	rc = pthread_create(&history_thread, NULL, writeHistoryThread, &airspace);
	if (rc) {
		printf("ERROR when creating history thread; Code is %d\n", rc);
	}
	// Send Aircraft Information and Collision Alerts to Display System
	pthread_t CompSys_thread;
	rc = pthread_create(&CompSys_thread, NULL, SendToCompSys, &airspace);
	if (rc) {
		printf("ERROR when creating SendToCompSys thread; Code is %d\n", rc);
	}
	pthread_join(CompSys_thread, NULL);
	pthread_join(OpConsole_thread, NULL);
	pthread_join(history_thread, NULL);

	return 0;
}
