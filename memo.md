## AHC029 memo

### 方針

1. 貪欲を突き詰める

- 評価値 $\frac{v+\alpha}{h}$ が一番高いプロジェクトを進める
  ※ $\alpha = 5*pow(2,L)$ として、h が小さい project が少し優遇されている
- $w-p$ が一番高いカードを購入する
- 最後 $50$ ターンは増資カード禁止
- キャンセルカードは持てるだけ持つ ⇒ $\frac{v}{h} < 1.02$ のプロジェクトを即交換
- 購入後金欠になりそうならカードは買わない (購入後動きやすいように)
- プロジェクト終了のタイミングで業務転換カードを使用 (初手手札圧圧迫防止)

#### 案

- 出来るだけターンを短縮するのが必要 ( 早く次の増資カードに繋げたい為 )
- 今あるカードの中で h が小さいものを積極的に回していくが、そのカードが無駄にならない程度の体力が残ってる
- カードの購入で Montecalro ? (購入はその場でしか出来ず、使用 part よりその場の判断が重要な為)
