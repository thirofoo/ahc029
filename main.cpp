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

#define TIME_LIMIT 1600
// #define TIME_LIMIT 15000

//-----------------以下から実装部分-----------------//

constexpr int T = 1000;
constexpr int MONTE_TURN = 50;
constexpr int TEST_NUM = 3000;
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
    bool is_test, pre_execute = false;
    vector<int> card_cnt;
    vector<Card> hand;
    vector<Project> project;
    vector<pair<Project,int>> sorted_project;
    int recent_use_hand_id, L = 0, pro_cnt;
    double expectation;
    long long money = 0;

    State() {}
    explicit State(vector<Card> _hand, vector<Project> _project, bool _is_test) : hand(_hand), project(_project), is_test(_is_test) {
        // 最初に期待値が最大・最小の project を求めておく
        bestWorstUpdate();
        card_cnt.assign(5,0);
        for(int i=0; i<N; i++) card_cnt[hand[i].t]++;
    }

    inline void bestWorstUpdate() {
        sorted_project.clear();
        for(int i=0; i<M; i++) sorted_project.emplace_back(make_pair(project[i],i));
        sort(sorted_project.begin(), sorted_project.end(), [&](const auto &a, const auto &b) {
            return (a.first.v+5*pow(2,L)) * b.first.h > (b.first.v+5*pow(2,L)) * a.first.h;
        });
        return;
    }

    inline void useCard(int card_id, int work, const vector<Project> &pro_tester) { // カード使用
        pre_execute = false;
        recent_use_hand_id = card_id;
        card_cnt[hand[card_id].t]--;

        if( hand[card_id].t == 0 ) { // 通常労働カード
            project[work].h -= hand[card_id].w;
            pre_execute |= (project[work].h <= 0);
            if( project[work].h <= 0 ) {
                money += project[work].v;
                if( is_test ) {
                    project[work] = pro_tester[(pro_cnt++)%pro_tester.size()];
                    project[work].h *= ( is_test ? pow(2,L) : 1 );
                    project[work].v *= ( is_test ? pow(2,L) : 1 );
                    bestWorstUpdate();
                }
            }
        }
        else if( hand[card_id].t == 1 ) { // 全力労働カード
            for(int i=0; i<M; i++) {
                project[i].h -= hand[card_id].w;
                pre_execute |= (project[i].h <= 0);
                if( project[i].h <= 0 ) {
                    money += project[i].v;
                    if( is_test ) {
                        project[i] = pro_tester[(pro_cnt++)%pro_tester.size()];
                        project[i].h *= pow(2,L);
                        project[i].v *= pow(2,L);
                    }
                }
            }
            if( is_test ) bestWorstUpdate();
        }
        else if( hand[card_id].t == 2 && is_test ) { // キャンセルカード
            project[work] = pro_tester[(pro_cnt++)%pro_tester.size()];
            project[work].h *= pow(2,L);
            project[work].v *= pow(2,L);
            bestWorstUpdate();
        }
        else if( hand[card_id].t == 3 && is_test ) { // 業務転換カード
            for(int i=0; i<M; i++) {
                project[i] = pro_tester[(pro_cnt++)%pro_tester.size()];
                project[i].h *= pow(2,L);
                project[i].v *= pow(2,L);
            }
            bestWorstUpdate();
        }
        else if( hand[card_id].t == 4 ) L++; // 増資カード
        return;
    }

    bool f = false;
    inline void buyCard(int card_id, const vector<Card> &cards) { // カード購入
        card_cnt[cards[card_id].t]++;
        hand[recent_use_hand_id] = cards[card_id];
        hand[recent_use_hand_id].w *= ( is_test & f ? pow(2,L) : 1 );
        hand[recent_use_hand_id].p *= ( is_test & f ? pow(2,L) : 1 );
        money -= hand[recent_use_hand_id].p;
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
            // キャンセルカード : 増資カードの次に優先で使用 ( 期待値低いものをキャンセル )
            if( hand[j].t == 2 && (double)sorted_project.back().first.v < (card_cnt[2] == N-1 ? 1.05 : 1.00) * sorted_project.back().first.h ) {
                op = j;
                break;
            }
            // 業務転換カード : 前回 best_project が遂行された場合のみ使用
            if( hand[j].t == 3 && pre_execute ) {
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
        // ※ ただその project より金がかかったカードは使用しない
        int sub = -1;
        if( hand[op].t == 0 ) {
            for(int i=0; i<M; i++) {
                // card がオーバー過ぎる価値を持ってる or オーバー過ぎる能力を持ってる時はスルー
                if( sorted_project[i].first.v > hand[op].p && hand[op].w <= sorted_project[i].first.h*3 ) {
                    sub = sorted_project[i].second;
                    break;
                }
            }
        }
        if( sub == -1 ) sub = ( hand[op].t == 0 ? sorted_project[0].second : ( hand[op].t == 2 ? sorted_project.back().second : 0 ) );
        
        return pair(op, sub);
    }

    inline int selectBuyCard(int turn, const vector<Card> &cand_card) {
        // ~~~~~~~~~~~~~~~~~~~~~~~ 貪欲で購入カード選択 ~~~~~~~~~~~~~~~~~~~~~~~ //
        // card : w-p が一番高いものを選択 ( 実は期待値が高いものは進展が遅く効率悪 )
        int best_card = 0, best_price = (int)1e16, semi_price = (int)1e16;
        double best_expectation = 0.0;
        for(int j=0; j<K; j++) {
            // そもそも購入不可 or 購入後金欠になりそうならスルー
            if( (double)cand_card[j].p * ( is_test ? pow(2,L) : 1 ) * 1.5 > money ) continue;
            if( cand_card[j].t == 0 || cand_card[j].t == 1 ) {
                if( best_expectation < cand_card[j].w * ( is_test ? pow(2,L) : 1 ) * (cand_card[j].t == 1 ? 0.8*M : 1) - cand_card[j].p * ( is_test ? pow(2,L) : 1 ) ) {
                    best_expectation = cand_card[j].w * ( is_test ? pow(2,L) : 1 ) * (cand_card[j].t == 1 ? 0.8*M : 1) - cand_card[j].p * ( is_test ? pow(2,L) : 1 );
                    best_card = j;
                }
            }
            else if( cand_card[j].t == 2 && cand_card[j].p * ( is_test ? pow(2,L) : 1 ) <= 8*pow(2,L) && card_cnt[0]+card_cnt[1] != 0 && turn < T - 50 ) {
                // キャンセルカードは購入可能なら優先で買う
                if( best_price == (int)1e16 && semi_price > cand_card[j].p * ( is_test ? pow(2,L) : 1 ) ) {
                    best_card = j;
                    best_expectation = 1e8;
                    semi_price = cand_card[j].p * ( is_test ? pow(2,L) : 1 );
                }
            }
            else if( cand_card[j].t == 4 && turn < T - 100 ) {
                // 増資カードが購入可能 ⇒ 最優先で買う
                // ※ ただし最後の 150 ターンは買わない
                if( best_price > cand_card[j].p * ( is_test ? pow(2,L) : 1 ) ) {
                    best_card = j;
                    best_expectation = 1e16;
                    best_price = cand_card[j].p * ( is_test ? pow(2,L) : 1 );
                }
            }
        }
        return best_card;
    }

    long long simulate(int turn, int card_id, const vector<Card> &first_card, const vector<vector<Card>> &tester, const vector<Project> &pro_tester) {
        f = false;
        buyCard(card_id, first_card); // 初手購入 part
        f = true;

        pro_cnt = 0;
        for(int i=turn+1; i<T; i++) {
            // 残りターンは全部貪欲
            auto [op, subject] = selectUseCard();
            useCard(op, subject, pro_tester);
            auto card_id = selectBuyCard(i, tester[i]);
            buyCard(card_id, tester[i]);
        }
        return money;
    }
};

struct Solver {
    State state;
    vector<Card> cards, cand_card;
    vector<Project> projects;
    vector<vector<Project>> project_tester;
    vector<vector<vector<Card>>> card_tester;

    vector<long long> scores;

    Solver() {
        this->input();
        state = State(cards, projects, false);
        cand_card.assign(K,Card{0,0,0});

        std::random_device seed;
	    std::mt19937 engine(seed()); 
        utility::mytm.CodeStart();        

        // 乱択 test 生成
        project_tester.assign(TEST_NUM, vector<Project>(T));
        card_tester.assign(TEST_NUM, vector<vector<Card>>(T));
        for(int i=0; i<TEST_NUM; i++) for(int j=T-MONTE_TURN; j<T; j++) for(int k=0; k<K; k++) {
            Card test_card;
            int rnd1 = rand_int()%EX;
            if( k == 0 ) { // 先頭の無料カード
                test_card.t = 0;
                test_card.w = 1;
                test_card.p = 0;
            }
            else if( rnd1 < X[0] ) { // 通常労働カード
                test_card.t = 0;
                test_card.w = rand_int()%50 + 1;
                std::normal_distribution<> dist((double)test_card.w, (double)test_card.w/3);
                test_card.p = max(1,min(10000,(int)round(dist(engine))));
            }
            else if( rnd1 < X[0] + X[1] ) { // 全力労働カード
                test_card.t = 1;
                test_card.w = rand_int()%50 + 1;
                std::normal_distribution<> dist((double)test_card.w, (double)test_card.w/3);
                test_card.p = max(1,min(10000,(int)round(dist(engine))));
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
            card_tester[i][j].emplace_back(test_card);
        }
        for(int i=0; i<TEST_NUM; i++) for(int j=0; j<T; j++) {
            double rnd = rand_double()*6.0 + 2.0;
            std::normal_distribution<> dist(rnd,0.5);
            project_tester[i][j] = Project{
                (int)(round(pow(2,rnd))),
                (int)(round(pow(2,max(0.0,min(10.0,dist(engine)))))),
            };
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

    inline int monte_carlo_method(int turn) {
        // (T-turn + α) / (∑(T-turn) + α') の比で時間を割り振り
        double rest_time = TIME_LIMIT - utility::mytm.elapsed();
        double rate = (double)(T - turn + 10) / ((T - turn) * ( T - turn + 21 ) / 2);
        double end_time = utility::mytm.elapsed() + rate * rest_time;
        int counter = TEST_NUM - 1;
        cerr << "Rate: " << rate << "\n" << flush;
        cerr << "Now Time: " << utility::mytm.elapsed() << " ms\n" << flush;
        cerr << "End Time: " << end_time << " ms\n" << flush;

        int cant_buy_cnt = 0;
        for(int i=0; i<K; i++) if( state.money < cand_card[i].p ) cant_buy_cnt++;
        if( cant_buy_cnt == K-1 ) return 0; // 買えるのが 0 枚目しか無い時は問答無用で 0 を返す

        scores.assign(K,0);
        while (utility::mytm.elapsed() < end_time && counter) {
            for(int i=0; i<K; i++) {
                if( state.money < cand_card[i].p ) continue;
                // 購入 part を Montecalro で評価
                State branch = state;
                branch.is_test = true;
                int test_score = branch.simulate(turn, i, cand_card, card_tester[counter], project_tester[counter]);
                scores[i] += test_score;
                // cerr << test_score << " ";
            }
            counter--;
        }
        cerr << "\nCounter: " << TEST_NUM-1-counter << "\n" << flush;
        int best_card = 0;
        long long best_score = -1;
        for(int i=0; i<K; i++) {
            cerr << scores[i] << " ";
            if( state.money < cand_card[i].p ) continue;
            if( best_score < scores[i] ) {
                best_score = scores[i];
                best_card = i;
            }
        }
        cerr << '\n';
        for(int i=0; i<K; i++) cerr << scores[i] / (TEST_NUM-1-counter) << " ";
        cerr << "\n\n" << flush;
        return best_card;
    }

    void solve(){
        // ~~~~~~~~~~~~~~~~~~~~ Montecalro method ~~~~~~~~~~~~~~~~~~~~ //
        for(int turn=0; turn<T; turn++) {
            cerr << "Turn " << turn << "\n" << flush;
            // cerr << "First hand: \n" << flush;
            // cerr << "L: " << state.L << "\n" << flush;
            // for(int i=0; i<state.hand.size(); i++) cerr << state.hand[i].t << " " << state.hand[i].w << " " << state.hand[i].p << "\n" << flush;
            // cerr << "First Project: \n" << flush;
            // for(int i=0; i<state.project.size(); i++) cerr << state.project[i].h << " " << state.project[i].v << "\n" << flush;
            // cerr << '\n' << flush;
            
            // ~~~~~~~~~~~~~~~~~~~~~~~ 基本処理ここから ~~~~~~~~~~~~~~~~~~~~~~~ //
            // Card 使用 part
            auto [op, subject] = state.selectUseCard();
            cout << op << " " << subject << '\n' << flush;
            state.useCard(op, subject, project_tester[turn]);
            
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
            int card_id;
            if( turn <= T-MONTE_TURN ) card_id = state.selectBuyCard(turn, cand_card);
            else card_id = monte_carlo_method(turn);
            // card_id = state.selectBuyCard(turn, cand_card);// 貪欲ver
            // card_id = monte_carlo_method(turn);
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
