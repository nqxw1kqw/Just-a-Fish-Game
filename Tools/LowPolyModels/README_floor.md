# 海底モデルの生成

ゲームは `Data/Model/LowPoly/Floor.x` を読み込みます。`GameConsts::FLOOR_USE_MODEL` でモデル表示を切り替え、読み込みに失敗した場合は従来の床を使用します。モデルは表示用で、地面の判定は Y=0 です。

## 再生成

Python 3.10 以降の標準ライブラリだけで生成できます。Blender や外部の `build_bosses.py` は不要です。リポジトリのルートから実行します。

```console
python Tools/LowPolyModels/build_floor.py
```

既定ではゲーム用の `Floor.x` を上書きします。別の場所へ書き出す場合は出力先を指定します。

```console
python Tools/LowPolyModels/build_floor.py --output Tools/LowPolyModels/out/Floor.x
```

スクリプト自身の場所から既定の出力先を求めるため、実行時の作業ディレクトリに依存しません。乱数シードは 7 で固定しています。`floor_mesh.py` が三角形、外向き法線、DirectX テキスト形式の出力を担当します。砂面の頂点順序と法線は上向きです。

同梱モデルは元の形状と装飾を保持して砂面と装飾の面・法線の向きを修正したものです。再生成では同じ配置規則を使いますが、自己完結したメッシュ生成関数へ移行したため、装飾の三角形分割は元データと完全には一致しません。
