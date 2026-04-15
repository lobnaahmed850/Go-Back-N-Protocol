#include <iostream>
#include <vector>
using namespace std;

//sequence numbers wrap around: 0,1,2,3,4,5,6,7,0,1,2,3,...
#define MAX_SEQ 7 // 0 : 7
#define N_SEQ (MAX_SEQ + 1) //total number of sequence numbers 
#define TIMEOUT 5  
int timer[N_SEQ]; // stores send time
int current_time = 0; 
int ack;
int WIN_SIZE;
int total_pkts;
int ack_expected = 0; // lower bound (start of window) indicates expected next ack frame
int next_frame_to_send = 0; // upper bound (the one after exactly window size)
//after timeout, next_frame_to_send = ack_expected
int nbuffered = 0; // number of outstanding frames (this buffer at sended side)
//outstanding frames: frames in window size not acknowledged yet
vector<int> sender_buf(N_SEQ, -1);

//! handling sequence numbers
//inc function returns next sequence number in circular order
int inc(int n) { //n: current sequence number
    return (n + 1) % N_SEQ;
    //ex: n=3, 4 % 8 = 4
    //ex: n=7, 8 % 8 = 0 (wrap around)
}

// check if b (ack) is between a (ack_expected) and c (next_frame_to_send)
bool between(int a, int b, int c) {
    if ((a <= b && b < c) || (c < a && a <= b) || (b < c && c < a))
        return true;
    return false;
}

// sending handling: normal send or resend after timeout
void send_data(int seq, bool retransmission = false) {
    if (retransmission)
        cout << "[RE-SEND] Frame " << seq <<"\n";
    else
        cout << "[SEND] Frame " << seq <<"\n";
}

// ACK simulation (waiting for real logic)
int simulate_ack(bool withBuffer, int last_sent) {
    if (withBuffer) {
        // individual ack
        cout << "Enter ACK (positive) or NACK (negative): ";
    }
    else {
        // cumulative ACK
        cout << "Enter cumulative ACK: ";
    }
    cin >> ack;
    return ack;
}

// Timeout simulation 
// retransmit all unack frames
bool simulate_timeout() {
    char choice;
    cout << "Timeout occurred? (y/n): ";
    cin >> choice;
    return (choice == 'y');
}

int main() {
    for (int i = 0; i < N_SEQ; i++)
        timer[i] = -1;

    cout << "Enter total packets: ";
    cin >> total_pkts;

    cout << "Enter window size: ";
    cin >> WIN_SIZE;
    if (WIN_SIZE >= N_SEQ) {
        cout << "Window size must be less than " << N_SEQ <<"!\n";
        return 0;
    }

    int scenario;
    cout << "Choose scenario:\n1.Receiver without buffer\n2.Receiver with buffer\n";
    cin >> scenario;
    bool withBuffer = (scenario == 2);


    int next_pkt_id = 0;

    while (ack_expected < total_pkts) {

    // Sliding window handling
    // condition of sending mrore frames: window has space and still have data to send
    while (nbuffered < WIN_SIZE && next_pkt_id < total_pkts) {
        // store packets in sender buffer before sending
        sender_buf[next_frame_to_send] = next_pkt_id;
        // sender actually sends frame
        send_data(next_frame_to_send);
        // start timer
        if (!withBuffer)
        timer[next_frame_to_send] = current_time; 
        //increase no of outstanding frames
        nbuffered++;
        next_pkt_id++;
        //move on to next seq no in circular order
        next_frame_to_send = inc(next_frame_to_send);
    }

    //simulate ack
    ack = simulate_ack(withBuffer, next_frame_to_send);
    if (withBuffer) {
        if (ack >= 0) {
            //individual ack handling: checking validity of ack
          //ack should be between ack_expected: 0, next_frame_to_send: 4
            if (between(ack_expected, ack, next_frame_to_send)) {
                cout << "ACK received for frame " << ack << "\n";
                //one frame is no longer outstanding
                nbuffered--;
                timer[ack] = -1;
                if (ack == ack_expected)
                    ack_expected = inc(ack_expected);
            }
            else {
                cout << "Invalid ACK ignored\n";
            }
        }
        else {
            // NACK
            int lost = -ack;
            cout << "NACK received for frame " << lost << "\n";
            send_data(lost, true);
        }
    }

    else { 
        // without buffer: cumulative ACK, receiver can send ack 2 directly means frames 0,1,2 received
        while (between(ack_expected, ack, next_frame_to_send)) {
            timer[ack_expected] = -1; // stop timer
            cout << "Cumulative ACK: " << ack_expected <<"\n";
            nbuffered--;
            ack_expected = inc(ack_expected);
        }
    }

    // simulate timeout
    if (!withBuffer) {
        bool timeout_flag = false;
        for (int i = 0; i < N_SEQ; i++) {
            if (timer[i] != -1 && (current_time - timer[i] >= TIMEOUT)) {
                cout << "Timeout detected for frame " << i << "\n";
                timeout_flag = true;
                break;
            }
        }
        if (timeout_flag) {
            cout << "Retransmitting all outstanding frames...\n";
            int temp = ack_expected;
            for (int i = 0; i < nbuffered; i++) {
                send_data(temp, true);
                timer[temp] = current_time; // restart timer
                temp = inc(temp);
            }
            next_frame_to_send = temp;
        }
    }
    current_time++;
    cout << "---------------------------------\n";
    }

    cout << "All packets sent successfully!\n";
}