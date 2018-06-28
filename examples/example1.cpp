//
// Created by dan on 4/29/18.
//

#include "../src/v4l2cxx.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>


int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}


bool keyboardhit(int &key_code)
{

    if (kbhit())
    {
        key_code = getchar(); //suppose to be from curses.h
        return true;
    }
    return false;
}

void callback_stdout_pipe(uint8_t *p_data, size_t len) {

    uint8_t outBuff[921600];
    util_v4l2::raw_to_rgb(p_data, 0, outBuff, 921600, 640 * 480, 10);
    fwrite(outBuff, 640*480*3, 1, stdout);
}

capture cap("/dev/video1", 640,480,pixel_format::V4L2CXX_PIX_FMT_YUYV,callback_stdout_pipe);

void debug_loop() {
    int exposure = 100;

    while (true) {
        int key_code;
        if (keyboardhit(key_code)) {
            if (key_code == '+') {
                exposure += 100;
                //std::cout << exposure << "\n";
                cap.set_exposure(exposure);

            } else if (key_code == '-') {
                if (exposure - 100 < 0) {
                    exposure = 0;
                } else {
                    exposure -= 100;
                    //std::cout << exposure << "\n";
                    cap.set_exposure(exposure);
                }

            }
        }
    }
}





int main() {

    ///////////////////////////////////////////////////////////////////////////////////////////////////




    cap.set_exposure(100);

    std::thread debug_thread(debug_loop);
    debug_thread.detach();
    cap.run();

    }


