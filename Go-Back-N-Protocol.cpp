#include <iostream>
#include <vector>
using namespace std;

#define MAX_SEQ 7
#define N_SEQ (MAX_SEQ + 1)

// circular increment
int inc(int n) {
    return (n + 1) % N_SEQ;
}

// check if b is between a and c (circular)
bool between(int a, int b, int c) {
    return ((a <= b && b < c) ||
        (c < a && a <= b) ||
        (b < c && c < a));
}


int main() {

    // receiver variables
    int frame_expected = 0;   // next frame needed
    int WINDOW_SIZE = 4;      // receiver window size

    vector<int> receiver_buf(N_SEQ, -1);   // buffer to store frames
    vector<bool> received(N_SEQ, false);   // mark received frames

    int seq;

    cout << "Receiver started...\n";

    while (true) {

        cout << "\nEnter received frame seq (-1 to stop): ";
        cin >> seq;

        if (seq == -1) break;

        // check if frame is inside receiver window
        if (between(frame_expected, seq, (frame_expected + WINDOW_SIZE) % N_SEQ)) {

            cout << "Frame " << seq << " is inside window\n";

            // if not already received
            if (!received[seq]) {
                receiver_buf[seq] = seq;
                received[seq] = true;
                cout << "Stored frame " << seq << " in buffer\n";
            }

            // send ACK
            cout << "ACK sent for frame " << seq << "\n";

            // deliver in-order frames
            while (received[frame_expected]) {
                cout << "Delivered frame " << frame_expected << " to network layer\n";
                received[frame_expected] = false;
                frame_expected = inc(frame_expected);
            }
        }
        else {
            cout << "Frame " << seq << " is OUTSIDE window -> ignored\n";
        }

        // send NACK if missing frame detected
        if (!received[frame_expected]) {
            cout << "NACK sent for frame " << frame_expected << "\n";
        }
    }

    cout << "Receiver stopped.\n";
}