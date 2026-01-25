#include <iostream>

int newest_sample = 1;

void updateNum(){
	newest_sample *= 2;
}

int main(int argc, char* argv[]){
	
	for (int i = 0; i < 10; i++) {
		updateNum();
		std::cout << "newest sample: " << newest_sample << std::endl;
	}

	return 0;
}
