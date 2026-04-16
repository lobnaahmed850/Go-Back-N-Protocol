#include <iostream>
#include <string>

using namespace std;

#define MAX_SEQ 7            // sequence numbers: 0..7
#define N_SEQ (MAX_SEQ + 1)  // total sequence numbers in circular space

// Return next sequence number in circular order.
int inc(int n) {
    return (n + 1) % N_SEQ;
}

// Check if b is between a and c in circular sequence space.
bool between(int a, int b, int c) {
    return ((a <= b && b < c) ||
            (c < a && a <= b) ||
            (b < c && c < a));
}

class Frame {
public:
    int seq;      // Sequence number of this frame
    int ack;      // ACK field
    string data;  // Payload/data

    Frame(int s, int a, const string& d) {
        seq = s;
        ack = a;
        data = d;
    }
};

class Receiver {
private:
    int frame_expected; // next in-order frame required

public:
    // Start by expecting frame 0
    Receiver() {
        frame_expected = 0;
    }

    // Process one incoming frame and return ACK value.
    int receiveFrame(const Frame& frame) {
        // 1) Print arrival information.
        cout << "Frame arrived -> seq: " << frame.seq
             << ", data: \"" << frame.data << "\"" << endl;

        // 2) Accept only in-order frame. Discard otherwise.
        if (frame.seq == frame_expected) {
            // Correct frame: accept and deliver.
            cout << "Accepted frame " << frame.seq << endl;
            cout << "Delivered data: " << frame.data << endl;

            // Move to next expected frame in circular sequence space.
            frame_expected = inc(frame_expected);
        } else {
            // Wrong order: discard (Go-Back-N receiver does NOT buffer).
            cout << "Discarded frame " << frame.seq
                 << " (expected " << frame_expected << ")." << endl;
        }

        // 3) Cumulative ACK rule from protocol:
        // ACK = (expectedFrame + MAX_SEQ) % (MAX_SEQ + 1)
        int ackValue = (frame_expected + MAX_SEQ) % N_SEQ;
        cout << "Sent ACK: " << ackValue << endl;
        cout << "-----------------------------------" << endl;

        // 4) Return ACK value so sender can process it.
        return ackValue;
    }
};

int main() {
    Receiver receiver;
    int seq;

    cout << "Receiver started (Go-Back-N, no buffer)...\n";

    while (true) {
        string payload;

        cout << "\nEnter received frame seq (-1 to stop): ";
        cin >> seq;
        if (seq == -1) break;

        cout << "Enter frame data (single word): ";
        cin >> payload;

        Frame in(seq, 0, payload);
        int ack_to_sender = receiver.receiveFrame(in);

        // ACK returned from receiver function (to sender side).
        cout << "Receiver returned ACK = " << ack_to_sender << endl;
    }

    cout << "Receiver stopped.\n";
    return 0;
}
