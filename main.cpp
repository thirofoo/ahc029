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
#define TIME_LIMIT 18000

//-----------------以下から実装部分-----------------//

constexpr int T = 1000;
constexpr int TEST_NUM = 800;
static const vector<int> X = {20/2, 10/2, 10/2, 5/2, 3/2}; // 隠しパラメータの期待値
constexpr int EX = (20 + 10 + 10 + 5 + 3) / 2;   // 隠しパラメータの期待値の総和
int N, M, K, _, L;

struct Card {
    // t:種類, w:労働力, p:購入額
    int t, w, p;
};

struct Project {
    // h:残務量, v:価値
    int h, v;
};

struct State{
    vector<Card> hand;
    vector<Project> project;
    int money = 0, total_get_money, pre_use_hand_id;

    State(){}
    explicit State(vector<Card> hand, vector<Project> project) : hand(hand), project(project) {}

    inline void useCard(int card_id, int work) { // カード使用
        pre_use_hand_id = card_id;
        if(hand[card_id].t == 0) { // 通常労働カード
            project[work].h -= hand[card_id].w;
            if(project[work].h <= 0) {
                money += project[work].v;
                total_get_money += project[work].v;
                double rnd = rand_double()*6.0 + 2.0;
                project[work] = Project{
                    (int)(round(pow(2,rnd)) * pow(2,L)),
                    (int)(round(pow(2,max(0.0,min(10.0,gaussian(rnd,0.5))))) * pow(2,L)),
                };
            }
        }
        else if(hand[card_id].t == 1) { // 全力労働カード
            for(int i=0; i<M; i++) {
                project[i].h -= hand[card_id].w;
                if(project[i].h <= 0) {
                    money += project[i].v;
                    total_get_money += project[i].v;
                    double rnd = rand_double()*6.0 + 2.0;
                    project[work] = Project{
                        (int)(round(pow(2,rnd)) * pow(2,L)),
                        (int)(round(pow(2,max(0.0,min(10.0,gaussian(rnd,0.5))))) * pow(2,L)),
                    };
                }
            }
        }
        else if(hand[card_id].t == 2) { // キャンセルカード
            double rnd = rand_double()*6.0 + 2.0;
            project[work] = Project{
                (int)(round(pow(2,rnd)) * pow(2,L)),
                (int)(round(pow(2,max(0.0,min(10.0,gaussian(rnd,0.5))))) * pow(2,L)),
            };
        }
        else if(hand[card_id].t == 3) { // 業務転換カード
            for(int i=0; i<M; i++) {
                double rnd = rand_double()*6.0 + 2.0;
                project[i] = Project{
                    (int)(round(pow(2,rnd)) * pow(2,L)),
                    (int)(round(pow(2,max(0.0,min(10.0,gaussian(rnd,0.5))))) * pow(2,L)),
                };
            }
        }
        else { // 増資カード
            L++;
        }
    }

    inline void buyCard(int card_id, const vector<Card> &cards) { // カード購入
        hand[pre_use_hand_id] = cards[card_id];
        money -= cards[card_id].p;
        return;
    }

    int simulate_use(int turn, int card_id, int sub, const vector<vector<Card>> &tester) {
        total_get_money = 0;
        useCard(card_id, sub);
        for(int i=turn; i<T; i++) {
            if( i != turn ) useCard(rand_int()%N, rand_int()%M);
            // 買えるカードを列挙 ⇒ 乱択で選択
            vector<int> can_buy;
            for(int j=0; j<K; j++) {
                if(money >= tester[i][j].p) {
                    can_buy.emplace_back(j);
                }
            }
            buyCard(can_buy[rand_int()%can_buy.size()], tester[i]);
        }
        return total_get_money;
    }

    int simulate_buy(int turn, int card_id, const vector<Card> &test, const vector<vector<Card>> &tester) {
        total_get_money = 0;
        buyCard(card_id, test);
        for(int i=turn+1; i<T; i++) {
            useCard(rand_int()%N, rand_int()%M);

            // 買えるカードを列挙 ⇒ 乱択で選択
            vector<int> can_buy;
            for(int j=0; j<K; j++) {
                if(money >= tester[i][j].p) {
                    can_buy.emplace_back(j);
                }
            }
            buyCard(can_buy[rand_int()%can_buy.size()], tester[i]);
        }
        return total_get_money;
    }
};

struct Solver {
    State state;
    vector<Card> cards, cand_card;
    vector<Project> project;
    vector<vector<vector<Card>>> tester;

    vector<vector<int>> scores_use;
    vector<int> scores_buy;

    Solver() {
        this->input();
        state = State(cards, project);
        cand_card.assign(N,Card{0,0,0});
        L = 0;

        // 乱択test生成
        for(int i=0; i<TEST_NUM; i++){
            vector<vector<Card>> test(T);
            for(int j=0; j<T; j++) {
                for(int k=0; k<K; k++) {
                    Card test_card;
                    int rnd1 = rand_int()%EX;
                    if( k == 0 ) { // 先頭の無料カード
                        test_card.t = 0;
                        test_card.w = pow(2,L);
                        test_card.p = 0;
                    }
                    else if( rnd1 < X[0] ) { // 通常労働カード
                        test_card.t = 0;
                        int rnd2 = rand_int()%50 + 1;
                        test_card.w = rnd2 * pow(2,L);
                        test_card.p = max(1,min(10000,(int)round(gaussian(rnd2, (double)rnd2/3)))) * pow(2,L);
                    }
                    else if( rnd1 < X[0] + X[1] ) { // 全力労働カード
                        test_card.t = 1;
                        int rnd2 = rand_int()%50 + 1;
                        test_card.w = rnd2 * pow(2,L);
                        test_card.p = max(1,min(10000,(int)round(gaussian(rnd2, (double)rnd2/3)))) * pow(2,L);
                    }
                    else if( rnd1 < X[0] + X[1] + X[2] ) { // キャンセルカード
                        test_card.t = 2;
                        test_card.w = 0;
                        test_card.p = rand_int()%11 * pow(2,L);
                    }
                    else if( rnd1 < X[0] + X[1] + X[2] + X[3] ) { // 業務転換カード
                        test_card.t = 3;
                        test_card.w = 0;
                        test_card.p = rand_int()%11 * pow(2,L);
                    }
                    else { // 増資カード
                        test_card.t = 4;
                        test_card.w = 0;
                        test_card.p = (rand_int()%801 + 200) * pow(2,L);
                    }
                    test[j].emplace_back(test_card);
                }
            }
            tester.emplace_back(test);
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
            project.emplace_back(Project{h,v});
        }
        return;
    }

    void output(){
        cout << state.money << endl;
        return;
    }

    inline pair<int,int> monte_carlo_method(int turn, bool used) {
        double rest_time = TIME_LIMIT - utility::mytm.elapsed();
        // (T-turn + α) / (2*∑(T-turn) + α') の比で時間を割り振り
        // ※ 分母の2 はカード使用 + カード購入の2回ある為
        double rate = (double)(T - turn + 10) / ((T - turn) * ( T - turn +1 ) + 10);
        double end_time = utility::mytm.elapsed() + rate * rest_time;
        int counter = TEST_NUM - 1;

        if( used ) scores_buy.assign(K,0);
        else scores_use.assign(N,vector<int>(M,0));
        
        while (utility::mytm.elapsed() < end_time && counter) {
            if( used ) { // カード購入part
                for(int i=0; i<K; i++) {
                    if( cand_card[i].p > state.money ) continue;
                    if( cand_card[i].t == 3 ) continue; // 業務転換カードは買わない
                    State branch = state;
                    scores_buy[i] += branch.simulate_buy(turn, i, cand_card, tester[counter]);
                }
            }
            else { // カード使用part
                for(int i=0; i<N; i++) {
                    for(int j=0; j<M; j++) {
                        // たまたま T%2 == 0 の時捜査対象が無い
                        if( state.hand[i].t%2 == 1 && j ) {
                            scores_use[i][j] = -1;
                            continue;
                        }
                        State branch = state;
                        scores_use[i][j] += branch.simulate_use(turn, i, j, tester[counter]);
                    }
                }
            }
            counter--;
        }

        cerr << "Get Money: \n" << flush;
        pair<int,int> best_op = pair(0,0); // 最低限初期解
        if( used ) { // カード購入part
            int best_score = 0;
            for(int i=0; i<K; i++) {
                cerr << i << " " << scores_buy[i] << '\n' << flush;
                if( best_score < scores_buy[i] ) {
                    best_score = scores_buy[i];
                    best_op = make_pair(i,-1);
                }
            }
        }
        else { // カード使用part
            int best_score = 0;
            for(int i=0; i<N; i++) {
                for(int j=0; j<M; j++) {
                    cerr << i << " " << j << " " << scores_use[i][j] << '\n' << flush;
                    if( best_score < scores_use[i][j] ) {
                        best_score = scores_use[i][j];
                        best_op = make_pair(i,j);
                    }
                }
            }
        }
        return best_op;
    }

    void solve(){
        utility::mytm.CodeStart();
        
        // Montecalro
        for(int i=0; i<T; i++) {
            cerr << "Turn " << i << "\n" << flush;
            cerr << "First hand: \n" << flush;
            for(int i=0; i<state.hand.size(); i++) cerr << state.hand[i].t << " " << state.hand[i].w << " " << state.hand[i].p << "\n" << flush;
            cerr << "First Project: \n" << flush;
            for(int i=0; i<state.project.size(); i++) cerr << state.project[i].h << " " << state.project[i].v << "\n" << flush;
            cerr << '\n' << flush;

            auto [op1, sub] = monte_carlo_method(i, false);
            state.useCard(op1, sub);
            
            cerr << "Simulate_use: " << state.pre_use_hand_id << " " << sub << '\n' << flush;
            cout << op1 << " " << sub << '\n' << flush;

            // 買えるカードを列挙 ⇒ 乱択で選択
            for(int j=0; j<M; j++) {
                int h, p; cin >> h >> p;
                state.project[j] = Project{h,p};
            }
            cin >> state.money;

            cerr << "Having Money: " << state.money << "\n" << flush;
            cerr << "can buy card: \n" << flush;

            for(int j=0; j<K; j++) {
                int t, w, p; cin >> t >> w >> p;
                cand_card[j] = Card{t,w,p};
                cerr << t << " " << w << " " << p << "\n" << flush;
            }

            auto [op2, _] = monte_carlo_method(i, true);
            state.buyCard(op2, cand_card);

            cerr << "Buy card: " << op2 << "\n" << flush;
            cerr << "Last hand: \n" << flush;
            for(int i=0; i<state.hand.size(); i++) cerr << state.hand[i].t << " " << state.hand[i].w << " " << state.hand[i].p << "\n" << flush;
            cerr << "Last Project: \n" << flush;
            for(int i=0; i<state.project.size(); i++) cerr << state.project[i].h << " " << state.project[i].v << "\n" << flush;
            cerr << "\n\n\n" << flush;
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
