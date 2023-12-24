#include <bits/stdc++.h>
using namespace std;

namespace utility {
    struct timer {
        chrono::system_clock::time_point start;
        // 開始時間を記録
        void CodeStart() {
            start = chrono::system_clock::now();
        }
        // 経過時間 (ms) を返す
        double elapsed() const {
        using namespace std::chrono;
            return (double)duration_cast<milliseconds>(system_clock::now() - start).count();
        }
    } mytm;
}

inline unsigned int rand_int() {
    static unsigned int tx = 123456789, ty=362436069, tz=521288629, tw=88675123;
    unsigned int tt = (tx^(tx<<11));
    tx = ty; ty = tz; tz = tw;
    return ( tw=(tw^(tw>>19))^(tt^(tt>>8)) );
}

inline double rand_double() {
    return (double)(rand_int()%(int)1e9)/1e9;
}

inline double gaussian(double mean, double stddev) {
    // 標準正規分布からの乱数生成（Box-Muller変換
    double z0 = sqrt(-2.0 * log(rand_double())) * cos(2.0 * M_PI * rand_double());
    // 平均と標準偏差の変換
    return mean + z0 * stddev;
}
#define TIME_LIMIT 1000
// #define TIME_LIMIT 15000

//-----------------以下から実装部分-----------------//

constexpr int T = 1000;
constexpr int TEST_NUM = 200;
static const vector<int> X = {20/2, 10/2, 10/2, 5/2, 3/2}; // 隠しパラメータの期待値
constexpr int EX = (20 + 10 + 10 + 5 + 3) / 2;   // 隠しパラメータの期待値の総和
int N, M, K, _;

struct Card {
    // t:種類, w:労働力, p:購入額
    int t, w, p;
};

struct Project {
    // h:残務量, v:価値
    int h, v;
};

struct State{
    bool is_test;
    vector<Card> hand;
    vector<Project> project;
    int money = 0, total_get_money = 0, recent_use_hand_id, pre_hand_t = -1, L = 0;

    double best_expectation, worst_expectation, expectation;
    int best_project, worst_project;

    State() {}
    explicit State(vector<Card> _hand, vector<Project> _project, bool _is_test) : hand(_hand), project(_project), is_test(_is_test) {
        // 最初に期待値が最大・最小の project を求めておく
        bestWorstUpdate();
    }

    inline void bestWorstUpdate() {
        best_expectation = 0, worst_expectation = 1e16;
        for(int i=0; i<M; i++) {
            expectation = (double)project[i].v / project[i].h;
            // Best : v / h が最大
            if( best_expectation < expectation ) {
                best_expectation = expectation;
                best_project = i;
            }
            // Worst : v / h が最小
            if( worst_expectation > expectation ) {
                worst_expectation = expectation;
                worst_project = i;
            }
        }
        return;
    }

    inline void useCard(int card_id, int work) { // カード使用
        recent_use_hand_id = card_id;
        pre_hand_t = hand[card_id].t;
        if( hand[card_id].t == 0 ) { // 通常労働カード
            project[work].h -= hand[card_id].w * ( is_test ? pow(2,L) : 1 );
            if( project[work].h <= 0 ) {
                money += project[work].v * ( is_test ? pow(2,L) : 1 );
                total_get_money += project[work].v * ( is_test ? pow(2,L) : 1 );
                if( is_test ) {
                    double rnd = rand_double()*6.0 + 2.0;
                    project[work] = Project{
                        (int)(round(pow(2,rnd)) * pow(2,L)),
                        (int)(round(pow(2,max(0.0,min(10.0,gaussian(rnd,0.5))))) * pow(2,L)),
                    };
                    bestWorstUpdate();
                }
            }
        }
        else if( hand[card_id].t == 1 ) { // 全力労働カード
            for(int i=0; i<M; i++) {
                project[i].h -= hand[card_id].w * ( is_test ? pow(2,L) : 1 );
                if( project[i].h <= 0 ) {
                    money += project[work].v * ( is_test ? pow(2,L) : 1 );
                    total_get_money += project[work].v * ( is_test ? pow(2,L) : 1 );
                    if( is_test ) {
                        double rnd = rand_double()*6.0 + 2.0;
                        project[work] = Project{
                            (int)(round(pow(2,rnd)) * pow(2,L)),
                            (int)(round(pow(2,max(0.0,min(10.0,gaussian(rnd,0.5))))) * pow(2,L)),
                        };
                    }
                }
            }
            if( is_test ) bestWorstUpdate();
        }
        else if( hand[card_id].t == 2 && is_test ) { // キャンセルカード
            double rnd = rand_double()*6.0 + 2.0;
            project[work] = Project{
                (int)(round(pow(2,rnd)) * pow(2,L)),
                (int)(round(pow(2,max(0.0,min(10.0,gaussian(rnd,0.5))))) * pow(2,L)),
            };
            bestWorstUpdate();
        }
        else if( hand[card_id].t == 3 && is_test ) { // 業務転換カード
            for(int i=0; i<M; i++) {
                double rnd = rand_double()*6.0 + 2.0;
                project[i] = Project{
                    (int)(round(pow(2,rnd)) * pow(2,L)),
                    (int)(round(pow(2,max(0.0,min(10.0,gaussian(rnd,0.5))))) * pow(2,L)),
                };
            }
            bestWorstUpdate();
        }
        else L++; // 増資カード
        return;
    }

    inline void buyCard(int card_id, const vector<Card> &cards) { // カード購入
        hand[recent_use_hand_id] = cards[card_id];
        money -= cards[card_id].p * ( is_test ? pow(2,L) : 1 );
        return;
    }

    inline pair<int,int> selectUseCard() {
        // ~~~~~~~~~~~~~~~~~~~~~~~ 貪欲で使用カード・宛先選択 ~~~~~~~~~~~~~~~~~~~~~~~ //
        int op = 0, best_card_expectation = 0;
        for(int j=0; j<N; j++) {
            // 増資カード : 最優先で使用
            if( hand[j].t == 4 ) {
                op = j;
                break;
            }
            // キャンセルカード : 増資カードの次に優先で使用
            if( hand[j].t == 2 ) {
                op = j;
                break;
            }
            // 業務転換カード : 前回 best_project が遂行された場合のみ使用
            if( hand[j].t == 3 && pre_hand_t == 0 ) {
                best_card_expectation = (int)1e16;
                op = j;
                continue;
            }
            // 労働カード : 労働力が高いものを使用
            int expectation = hand[j].w * (hand[j].t == 1 ? M : 1);
            if(  best_card_expectation < expectation ) {
                best_card_expectation = expectation;
                op = j;
            }
        }
        // 労働カードの場合 : best_project を選択, 業務転換カードの場合 : worst_project を選択
        return pair(op, ( hand[op].t == 0 ? best_project : ( hand[op].t == 2 ? worst_project : 0 ) ));
    }

    inline int selectBuyCard(int turn, const vector<Card> &cand_card) {
        // ~~~~~~~~~~~~~~~~~~~~~~~ 貪欲で購入カード選択 ~~~~~~~~~~~~~~~~~~~~~~~ //
        // card : w-p が一番高いものを選択 ( 実は期待値が高いものは進展が遅く効率悪 )

        // 最後 100 ターンは 0枚目のカードしか買わない
        if( turn > T - 100 ) return 0;

        int best_card = 0, best_price = (int)1e16;
        double best_expectation = 0;
        for(int j=0; j<K; j++) {
            if( cand_card[j].p > money ) continue; // そもそも購入不可は論外
            if( cand_card[j].t == 0 || cand_card[j].t == 1 ) {
                if( best_expectation < cand_card[j].w * (cand_card[j].t == 1 ? M : 1) - cand_card[j].p ) {
                    best_expectation = cand_card[j].w * (cand_card[j].t == 1 ? M : 1) - cand_card[j].p;
                    best_card = j;
                }
            }
            else if( cand_card[j].t == 4 && turn < T - 200 ) {
                // 増資カードが購入可能 ⇒ 最優先で買う
                // ※ ただし最後の 200 ターンは買わない
                if( best_price > cand_card[j].p ) {
                    best_card = j;
                    best_expectation = 1e16;
                    best_price = cand_card[j].p;
                }
            }
        }
        return best_card;
    }

    int simulate(int turn, int card_id, const vector<vector<Card>> &tester) {
        total_get_money = 0;
        useCard(card_id, ( hand[card_id].t == 0 ? best_project : ( hand[card_id].t == 2 ? worst_project : 0 ) ));
        for(int i=turn; i<T; i++) {
            // 貪欲で選択 ⇒ 最後まで遂行して期待値算出
            if( i != turn ) {
                auto [op, subject] = selectUseCard();
                useCard(op, subject);
            }
            auto card_id = selectBuyCard(i, tester[i]);
            buyCard(card_id, tester[i]);
        }
        return total_get_money;
    }
};

struct Solver {
    State state;
    vector<Card> cards, cand_card;
    vector<Project> projects;
    vector<vector<vector<Card>>> tester;

    vector<int> scores;

    Solver() {
        this->input();
        state = State(cards, projects, false);
        cand_card.assign(K,Card{0,0,0});

        // 乱択 test 生成
        tester.assign(TEST_NUM, vector<vector<Card>>(T));
        for(int i=0; i<TEST_NUM; i++) for(int j=0; j<T; j++) for(int k=0; k<K; k++) {
            Card test_card;
            int rnd1 = rand_int()%EX;
            if( k == 0 ) { // 先頭の無料カード
                test_card.t = 0;
                test_card.w = 1;
                test_card.p = 0;
            }
            else if( rnd1 < X[0] ) { // 通常労働カード
                test_card.t = 0;
                int rnd2 = rand_int()%50 + 1;
                test_card.w = rnd2;
                test_card.p = max(1,min(10000,(int)round(gaussian(rnd2, (double)rnd2/3))));
            }
            else if( rnd1 < X[0] + X[1] ) { // 全力労働カード
                test_card.t = 1;
                int rnd2 = rand_int()%50 + 1;
                test_card.w = rnd2;
                test_card.p = max(1,min(10000,(int)round(gaussian(rnd2, (double)rnd2/3))));
            }
            else if( rnd1 < X[0] + X[1] + X[2] ) { // キャンセルカード
                test_card.t = 2;
                test_card.w = 0;
                test_card.p = rand_int()%11;
            }
            else if( rnd1 < X[0] + X[1] + X[2] + X[3] ) { // 業務転換カード
                test_card.t = 3;
                test_card.w = 0;
                test_card.p = rand_int()%11;
            }
            else { // 増資カード
                test_card.t = 4;
                test_card.w = 0;
                test_card.p = (rand_int()%801 + 200);
            }
            tester[i][j].emplace_back(test_card);
        }
    }

    void input(){
        cin >> N >> M >> K >> _;
        for(int i=0; i<N; i++) {
            int t, w; cin >> t >> w;
            cards.emplace_back(Card{t,w,0});
        }
        for(int i=0; i<M; i++) {
            int h, v; cin >> h >> v;
            projects.emplace_back(Project{h,v});
        }
        return;
    }

    void output(){
        cout << state.money << endl;
        // cerr << "Time Passed: " << utility::mytm.elapsed() << " ms\n" << flush;
        return;
    }

    inline pair<int,int> monte_carlo_method(int turn) {
        double rest_time = TIME_LIMIT - utility::mytm.elapsed();
        // (T-turn + α) / (∑(T-turn) + α') の比で時間を割り振り
        double rate = (double)(T - turn + 10) / ((T - turn) * ( T - turn + 1 ) / 2 + 10);
        double end_time = utility::mytm.elapsed() + rate * rest_time;
        int counter = TEST_NUM - 1;

        scores.assign(N,0);
        while (utility::mytm.elapsed() < end_time && counter) {
            for(int i=0; i<N; i++) {
                // 何の Card を使うかだけ全探索 ⇒ 残りは全貪欲で Montecalro
                State branch = state;
                branch.is_test = true;
                scores[i] += branch.simulate(turn, i, tester[counter]);
            }
            counter--;
        }
        cerr << "Counter: " << TEST_NUM-1-counter << "\n" << flush;

        pair<int,int> best_op = pair(0,0); // 最低限初期解
        int best_score = -1;
        for(int i=0; i<N; i++) {
            if( best_score < scores[i] ) {
                best_score = scores[i];
                best_op = make_pair(i, ( state.hand[i].t == 0 ? state.best_project : ( state.hand[i].t == 2 ? state.worst_project : 0 )));
            }
        }
        return best_op;
    }

    void solve(){
        utility::mytm.CodeStart();
        // ~~~~~~~~~~~~~~~~~~~~ Montecalro method ~~~~~~~~~~~~~~~~~~~~ //
        for(int turn=0; turn<T; turn++) {
            // cerr << "Turn " << turn << "\n" << flush;
            // cerr << "First hand: \n" << flush;
            // for(int i=0; i<state.hand.size(); i++) cerr << state.hand[i].t << " " << state.hand[i].w << " " << state.hand[i].p << "\n" << flush;
            // cerr << "First Project: \n" << flush;
            // for(int i=0; i<state.project.size(); i++) cerr << state.project[i].h << " " << state.project[i].v << "\n" << flush;
            // cerr << '\n' << flush;
            // cerr << "Best Project: " << state.best_project << "\n" << flush;
            // cerr << "Worst Project: " << state.worst_project << "\n" << flush;
            
            // ~~~~~~~~~~~~~~~~~~~~~~~ 基本処理ここから ~~~~~~~~~~~~~~~~~~~~~~~ //
            // Card 使用 part
            auto [op, subject] = state.selectUseCard(); // 貪欲ver
            // auto [op, subject] = monte_carlo_method(turn);
            cout << op << " " << subject << '\n' << flush;
            state.useCard(op, subject);
            
            // 状況確認 part
            for(int j=0; j<M; j++) {
                int h, p; cin >> h >> p;
                state.project[j] = Project{h,p};
            }
            state.bestWorstUpdate();
            cin >> state.money;
            for(int j=0; j<K; j++) {
                int t, w, p; cin >> t >> w >> p;
                cand_card[j] = Card{t,w,p};
            }

            // Card 購入 part
            auto card_id = state.selectBuyCard(turn, cand_card);
            cout << card_id << '\n' << flush;
            state.buyCard(card_id, cand_card);
            // ~~~~~~~~~~~~~~~~~~~~~~~ 基本処理ここまで ~~~~~~~~~~~~~~~~~~~~~~~ //

            // cerr << "Buy card: " << card_id << '\n' << flush;
            // cerr << "Last hand: \n" << flush;
            // for(int i=0; i<state.hand.size(); i++) cerr << state.hand[i].t << " " << state.hand[i].w << " " << state.hand[i].p << "\n" << flush;
            // cerr << "Last Project: \n" << flush;
            // for(int i=0; i<state.project.size(); i++) cerr << state.project[i].h << " " << state.project[i].v << "\n" << flush;
            // cerr << "\n\n" << flush;
        }
        return;
    }
};

int main(){
    cin.tie(0);
    ios_base::sync_with_stdio(false);

    Solver solver;
    solver.solve();
    solver.output();
    
    return 0;
}
