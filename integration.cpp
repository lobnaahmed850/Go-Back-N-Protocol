#include <iostream>
#include <vector>
#include <string>
using namespace std;

//todo piggyback ack not implemented (send data + ack simultaneously)

//sequence numbers wrap around: 0,1,2,3,4,5,6,7,0,1,2,3,...
#define MAX_SEQ 7 // 0 : 7
#define N_SEQ (MAX_SEQ + 1) //total number of sequence numbers 
#define TIMEOUT 3 // timeout interval for transmission

//! handling sequence numbers
//inc function returns next sequence number in circular order
int inc(int n) { 
    return (n + 1) % N_SEQ;
    //ex: n=3, 4 % 8 = 4
    //ex: n=7, 8 % 8 = 0 (wrap around)
}

bool between(int a, int b, int c) {
    // check if b (ack) is between a (ack_expected) and c (next_frame_to_send)
    return ((a <= b && b < c) || (c < a && a <= b) || (b < c && c < a));
}

struct Frame {
    int seq;
    string data;
    int checksum;
};

int compute_checksum(int seq, string data) {
    int sum = seq;
    for (char c : data)
        sum += c;
    return sum;
}

bool is_corrupted() {
    return rand() % 5 == 0;  //random corruption
}

void send_data(int seq, int pkt_id, bool retransmission = false) {
    Frame f;
    f.seq = seq;
    f.data = to_string(pkt_id);
    f.checksum = compute_checksum(f.seq, f.data);
    if (retransmission)
        cout << "Resend Frame " << seq << " (Pkt ID: " << pkt_id << ")\n";
    else
        cout << "Send Frame " << seq << " (Pkt ID: " << pkt_id << ")\n";
}

int simulate_ack(bool withBuffer) {
    int ack_input;
    if (withBuffer) 
        cout << "Enter ACK (positive) or NACK (negative): ";
    //NACK -3 for frame 3
    else 
        cout << "Enter cumulative ACK: ";
    cin >> ack_input;
    return ack_input;
}

class Receiver {
private:
    int frame_expected;
    int win_size;
    bool has_buffer;
    vector<bool> received_map;

public:
    Receiver(bool withBuffer, int ws) : frame_expected(0), win_size(ws), has_buffer(withBuffer) {
        received_map.assign(N_SEQ, false);
    }

    void processFrame(Frame f) {
        int received_checksum = f.checksum;
        int calculated_checksum = compute_checksum(f.seq, f.data);

        if (received_checksum != calculated_checksum) {
            cout << "checksum error. frame discarded!\n";
            return; 
        }
        cout << "Arrived: Seq " << f.seq;

        if (has_buffer) {
            // Logic with Buffer (Selective-like behavior)
            if (between(frame_expected, f.seq, (frame_expected + win_size) % N_SEQ)) {
                if (!received_map[f.seq]) {
                    received_map[f.seq] = true;
                    cout << " -> Buffered.";
                }
                // Slide window for in-order frames
                while (received_map[frame_expected]) {
                    received_map[frame_expected] = false;
                    frame_expected = inc(frame_expected);
                }
            }
            else 
                cout << " -> OUTSIDE Window. Ignored.";
        }
        else {
            // No Buffer
            if (f.seq == frame_expected) {
                cout << " -> Accepted.";
                frame_expected = inc(frame_expected);
            }
            else 
                cout << " -> Out of order. Discarded (Expected " << frame_expected << ")";
        }
        cout << "\n";
    }
};

// --- Main Simulation ---
int main() {
    int total_pkts, win_size, scenario;
    int current_time = 0;
    int ack_expected = 0; // lower bound (start of window) indicates expected next ack frame
    int next_frame_to_send = 0; // upper bound (the one after exactly window size)
    int next_pkt_id = 0;
    int nbuffered = 0; // number of outstanding frames (this buffer at sender side)
    //outstanding frames: frames in window size not acknowledged yet
    vector<int> timer(N_SEQ, -1); // stores send time
    vector<int> sender_buf(N_SEQ, -1);

    cout << "Go-Back-N Protocol :)\n";
    cout << "Enter total packets to send: ";
    cin >> total_pkts;

    do {
        cout << "Enter window size: ";
        cin >> win_size;
        if (win_size >= N_SEQ) 
            cout << "Window size is too large for the sequence space!\n";
        else if (win_size <= 0) 
            cout << "Window size must be >= 1\n";
    } while (win_size >= N_SEQ || win_size <= 0);

    cout << "Choose scenario:\n1.Receiver without buffer\n2.Receiver with buffer\n";
    cin >> scenario;

    bool withBuffer = (scenario == 2);
    Receiver receiver(withBuffer, win_size);

    while (ack_expected < total_pkts) {

        // Sliding window handling
       // condition of sending mrore frames: window has space and still has data to send
        while (nbuffered < win_size && next_pkt_id < total_pkts) {
            // store packets in sender buffer before sending
            sender_buf[next_frame_to_send] = next_pkt_id; //from network layer
            // sender actually sends frame to physial layer
            send_data(next_frame_to_send, sender_buf[next_frame_to_send]); 
            if(!withBuffer)
            timer[next_frame_to_send] = current_time;
            nbuffered++; //expand window
            next_pkt_id++;
            next_frame_to_send = inc(next_frame_to_send);
        }

        // 2. Simulation Interaction
        int user_seq;
        cout << "\nCurrent Time: " << current_time << "\n";
        cout << "Enter arriving frame sequence number (-1 = no arrival):";
        cin >> user_seq;

        if (user_seq >= 0 && user_seq < N_SEQ) {
            if (sender_buf[user_seq] == -1) {
                cout << "Frame not sent yet!\n";
                continue;
            }

            Frame f;
            f.seq = user_seq;
            f.data = to_string(sender_buf[user_seq]);
            //recombute checksum at sender
            f.checksum = compute_checksum(f.seq, f.data);
            if (is_corrupted()) {
                cout << "Frame corrupted!\n";
                f.data = "error";
            }
            //receiver processes whether frame is corrupted
            receiver.processFrame(f);
            int response = simulate_ack(withBuffer);

            if (withBuffer) {
                if (response >= 0 && between(ack_expected% N_SEQ, response, next_frame_to_send% N_SEQ)) {
                    cout << "Individual ACK for " << response << "\n";
                    while (between(ack_expected % N_SEQ, response, next_frame_to_send)) {
                        ack_expected = inc(ack_expected);
                        nbuffered--;
                    }
                }
                else if (response < 0) {
                    int nack_frame = -response;
                    cout << "NACK for " << -response << "\n";
                    if (sender_buf[nack_frame] != -1) 
                        send_data(nack_frame, sender_buf[nack_frame], true);
                }
            }
            else {
                // without buffer: cumulative ACK, receiver can send ack 2 directly means frames 0,1,2 received
                //frame arrival
                while (between(ack_expected % N_SEQ, response, next_frame_to_send)) {
                    cout << "Cumulative ACK up to: " << (ack_expected % N_SEQ) << "\n";
                    timer[ack_expected % N_SEQ] = -1; //stop timer at ack
                    nbuffered--;
                    ack_expected = inc(ack_expected);
                    if (ack_expected >= total_pkts) break;
                }
            }
        }
            if (!withBuffer) {
                //detect an unack frame (only oldest unack frame matters)
                bool timeout_occurred = false;
                int oldest = ack_expected % N_SEQ;
                if (timer[oldest] != -1 && current_time - timer[oldest] >= TIMEOUT) {
                    timeout_occurred = true;
                }

                if (timeout_occurred) {
                    //timeout event
                    cout << "Retransmitting all outstanding frames...\n";
                    int temp = ack_expected % N_SEQ;
                    for (int i = 0; i < nbuffered; i++) {
                        send_data(temp, sender_buf[temp], true);
                        timer[temp] = current_time; //reset timer when resent
                        temp = inc(temp);
                    }
                }
            }

        current_time++;
        cout << "\nPackets Acked: " << ack_expected << "/" << total_pkts << "\n";
    }

    cout << "\nSUCCESS :) All " << total_pkts << " packets acknowledged\n";
    return 0;
}