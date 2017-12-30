


static uint32_t currentTime = 0;
static uint32_t previousTime = 0;
static uint16_t cycleTime = 0;     // this is the number in micro second to achieve a full loop, it can differ a little and is taken into account in the PID loop

static uint32_t __fourHundredHzLoop = 0;
static uint32_t __twoHundredHzLoop = 0;
static uint32_t __oneHundredHzLoop = 0;
static uint32_t __fiftyHzLoop = 0;
static uint32_t __twentyFiveHzLoop = 0;
static uint32_t __twentyHzLoop = 0;
static uint32_t __tenHzLoop = 0;
static uint32_t __fiveHzLoop = 0;
static uint32_t __oneHzLoop = 0;

void initTimings(){
	previousTime = micros();

	__fourHundredHzLoop = previousTime;
	__twoHundredHzLoop = previousTime;
	__oneHundredHzLoop = previousTime;
	__fiftyHzLoop = previousTime;
	__twentyFiveHzLoop = previousTime;
	__twentyHzLoop = previousTime;
	__tenHzLoop = previousTime;
	__fiveHzLoop = previousTime;
	__oneHzLoop = previousTime;
}

void updateTimings(){
	currentTime = micros();
	cycleTime = currentTime - previousTime;
	previousTime = currentTime;
}

void setPreviousTime(){
	previousTime = micros();
}

bool is400HzLoop(){
	if (currentTime > __fourHundredHzLoop){
		__fourHundredHzLoop = currentTime + 2500;
		return true;
	}

	return false;
}

bool is200HzLoop(){
	if (currentTime > __twoHundredHzLoop){
		__twoHundredHzLoop = currentTime + 5000;
		return true;
	}

	return false;
}

bool is100HzLoop(){
	if (currentTime > __oneHundredHzLoop){
		__oneHundredHzLoop = currentTime + 10000;
		return true;
	}

	return false;
}

bool is50HzLoop(){	
	if (currentTime > __fiftyHzLoop){
		__fiftyHzLoop = currentTime + 20000;
		return true;
	}

	return false;
}

bool is25HzLoop(){
	if (currentTime > __twentyFiveHzLoop){
		__twentyFiveHzLoop = currentTime + 40000;
		return true;
	}

	return false;
}

bool is20HzLoop() {
	if (currentTime > __twentyHzLoop){
		__twentyHzLoop = currentTime + 50000;
		return true;
	}

	return false;
}

bool is5HzLoop() {
	if (currentTime > __fiveHzLoop){
		__fiveHzLoop = currentTime + 200000;
		return true;
	}

	return false;
}

bool is10HzLoop() {
	if (currentTime > __tenHzLoop){
		__tenHzLoop = currentTime + 100000;
		return true;
	}

	return false;
}

bool is1HzLoop() {
	if (currentTime > __oneHzLoop){
		__oneHzLoop = currentTime + 1000000;
		return true;
	}

	return false;
}

void timerHandler(){
	currentTime = micros();

	if (is400HzLoop())
		fourHundredHzLoop();

	if (is200HzLoop())
		twoHundredHzLoop();

	if (is100HzLoop())
		oneHundredHzLoop();

	if (is50HzLoop())
		fiftyHzLoop();

	if (is20HzLoop())
		twentyHzLoop();

	if (is10HzLoop())
		tenHzLoop();

	if (is5HzLoop())
		fiveHzLoop();

	if (is1HzLoop())
		oneHzLoop();
}

uint32_t getCurrentTime(){
	return currentTime;
}

uint16_t getCycleTime(){
	return cycleTime;
}