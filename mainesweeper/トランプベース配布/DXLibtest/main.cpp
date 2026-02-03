#include "DxLib.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	SetOutApplicationLogValidFlag(false);//ログ出力オフ
	ChangeWindowMode(TRUE); //ウィンドウモード切り替え
	SetGraphMode(480,433, 32); //ウィンドウサイズ

	if (DxLib_Init() == -1) { //DXライブラリ初期化処理
		return -1;			  //エラーが起きたら直ちに終了
	}

	SetDrawScreen(DX_SCREEN_BACK); //描画先を裏画面に変更
	SetWindowText("toranpu"); //ウィンドウの名前

	//画像---------------------
	int gh;//背景
	int gh_turf;//捲れるマス
	int empty;//数字なしマス
	int one;//1
	int two;//2
	int three;//3
	int four;//4
	int five;//5
	int six;//6
	int seven;//7
	int gh_frag;//旗
	int mine;//爆弾
	
	int startTileX = 0;//タイル座標
	int startTileY = 50;

	const int MAP_WIDTH = 15;
	const int MAP_HEIGHT = 12;

	const int MINE_COUNT = 30;

	//旗カウント
	int fr = 30;

	int timecount = 0;  //0:止まっている 1:動いている
	int startTime = 0;  //タイマー開始した瞬間
	int TimeMs = 0;     //経過ミリ秒（表示用）

	// タイルサイズを決める（32×32なら画面に収まる）
	const int TILE_W = 32;
	const int TILE_H = 32;

	int tileImg[5] = { -1, -1, -1, -1, -1 };//タイル使用数

	//画像読み込み
	gh = LoadGraph("image\\back.png");
	gh_frag = LoadGraph("image\\frag.png");

	//タイルになんの画像を描画するのか
	tileImg[1] = LoadGraph("image/turf.png");
	tileImg[2] = LoadGraph("image/empty.png");

	//数字画像読み込み（パスは自分のファイル名に合わせて）
	one = LoadGraph("image\\one.png");
	two = LoadGraph("image\\two.png");
	three = LoadGraph("image\\three.png");
	four = LoadGraph("image\\four.png");
	five = LoadGraph("image\\five.png");
	six = LoadGraph("image\\six.png");
	seven = LoadGraph("image\\seven.png");

	mine = LoadGraph("image\\mine.png");

	int numImg[8] = { -1,-1,-1,-1,-1,-1,-1,-1 }; // 0は使わない、1～7だけ使う
	numImg[1] = one;
	numImg[2] = two;
	numImg[3] = three;
	numImg[4] = four;
	numImg[5] = five;
	numImg[6] = six;
	numImg[7] = seven;

	// 読み込み失敗チェック（任意）
	if (gh == -1 || gh_frag == -1 || tileImg[1] == -1) 
	{
		DxLib_End();
		return -1;
	}

	//マップの描画
	int tempMap[MAP_HEIGHT][MAP_WIDTH] =
	{
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	};

	int mineMap[MAP_HEIGHT][MAP_WIDTH] = { 0 }; // 1なら地雷
	int numMap[MAP_HEIGHT][MAP_WIDTH] = { 0 }; // 周囲地雷数(0-8)
	int openMap[MAP_HEIGHT][MAP_WIDTH] = { 0 }; // 1なら開いてる

	int flagMap[MAP_HEIGHT][MAP_WIDTH] = { 0 }; // 旗がある:1 ない:0


	//-------------------地雷＆数字生成----------------------------------
	//範囲チェック
	auto InRange = [&](int x, int y)
	{
		return (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT);
	};

	//最初クリックの3×3は安全にして地雷配置
	auto PlaceMines = [&](int cx, int cy)
	{
		SRand(GetNowCount());
		int placed = 0;

		while (placed < MINE_COUNT)
		{
			int x = GetRand(MAP_WIDTH - 1);
			int y = GetRand(MAP_HEIGHT - 1);

			//最初のクリック周辺3×3は地雷を置かない
			if (x >= cx - 1 && x <= cx + 1 && y >= cy - 1 && y <= cy + 1) continue;

			//重複防止
			if (mineMap[y][x] == 1) continue;

			mineMap[y][x] = 1;
			placed++;
		}
    };

	//周囲地雷数（数字）を作る
	auto CalcNumbers = [&]()
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			for (int x = 0; x < MAP_WIDTH; x++)
			{
				if (mineMap[y][x] == 1) { numMap[y][x] = -1; continue; }

				int cnt = 0;
				for (int dy = -1; dy <= 1; dy++)
				{
					for (int dx = -1; dx <= 1; dx++)
					{
						if (dx == 0 && dy == 0) continue;
						int nx = x + dx, ny = y + dy;
						if (!InRange(nx, ny)) continue;
						if (mineMap[ny][nx] == 1) cnt++;
					}
				}
				numMap[y][x] = cnt; //0～8
			}
		}
	};

	bool gameOver = false;

	bool gameClear = false; // クリアしたら true

	//-------------------空白連鎖めくり----------------------------------
	//空白(0)なら周囲8マスを広げて開く（数字はそこで止まる）
	auto Reveal = [&](int sx, int sy)
	{
		//範囲外なら終了
		if (!InRange(sx, sy)) return;

		//旗があるなら開けない
		if (flagMap[sy][sx] == 1) return;

		//すでに開いてるなら終了
		if (openMap[sy][sx] == 1) return;

		//地雷なら（今はとりあえず踏んだマスだけ開く）
		if (mineMap[sy][sx] == 1)
		{
			openMap[sy][sx] = 1;
			tempMap[sy][sx] = 2;
			gameOver = true;
			return;
		}

		//BFS（キュー）で連鎖
		int qx[MAP_WIDTH * MAP_HEIGHT];
		int qy[MAP_WIDTH * MAP_HEIGHT];
		int head = 0, tail = 0;

		qx[tail] = sx; qy[tail] = sy; tail++;

		while (head < tail)
		{
			int x = qx[head];
			int y = qy[head];
			head++;

			//範囲外や開いてるのは飛ばす
			if (!InRange(x, y)) continue;
			if (openMap[y][x] == 1) continue;
			if (flagMap[y][x] == 1) continue; //旗は開けない

			//まず開く
			openMap[y][x] = 1;
			tempMap[y][x] = 2;

			//数字(1以上)ならここで止める
			if (numMap[y][x] > 0) continue;

			//空白(0)なら周囲8マスをキューに追加
			for (int dy = -1; dy <= 1; dy++)
			{
				for (int dx = -1; dx <= 1; dx++)
				{
					if (dx == 0 && dy == 0) continue;
					int nx = x + dx;
					int ny = y + dy;

					if (!InRange(nx, ny)) continue;
					if (openMap[ny][nx] == 1) continue;
					if (mineMap[ny][nx] == 1) continue; //地雷は開かない

					qx[tail] = nx;
					qy[tail] = ny;
					tail++;
				}
			}
		}
	};


	//-------------------クリア判定----------------------------------
	//地雷じゃないマスが全部開いたらクリア
	auto CheckClear = [&]()
	{
		int openedSafe = 0; // 開いた「地雷じゃない」マス数
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			for (int x = 0; x < MAP_WIDTH; x++)
			{
				if (mineMap[y][x] == 0 && openMap[y][x] == 1)
				{
					openedSafe++;
				}
			}
		}

		int totalSafe = MAP_WIDTH * MAP_HEIGHT - MINE_COUNT;
		if (openedSafe >= totalSafe)
		{
			gameClear = true;
		}
	};


	bool firstClick = true; // 最初のクリック判定

	// マウス座標取得（& と <= は C++ 記法で）
	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	int maemouse = 0;

	//リセット用
	int prevR = 0;

	//画像の分割読み込み
	//LoadDivGraph(画像ファイルポインタ、分割総数、横分割数、縦分割数、横サイズ、縦サイズ、保存配列ポインタ)
	//トランプの横サイズ：64、縦サイズ：92

	while (1) 
	{
		//裏画面のデータを全て削除
		ClearDrawScreen();

		//処理----------------------------------------------------------------

		//マウスカーソルの位置を取得
		GetMousePoint(&mouseX, &mouseY);

		for (int y = -1; y <= 1; y++)
		{
			for (int x = -1; x <= 1; x++)
			{

			}
		}

		//背景
		DrawGraph(0, 0, gh, true);//背景を先に描画しないと背景により全て隠れてしまう

		// マップ描画：tempMap の値（= タイルID）に応じてタイルを並べる
		for (int ty = 0; ty < MAP_HEIGHT; ++ty)
		{
			for (int tx = 0; tx < MAP_WIDTH; ++tx)
			{
				int id = tempMap[ty][tx];  //←ここでtempMapを使う
				if (id == 0) continue;     //0は何も描かない扱い

				// 範囲安全策（未ロードIDを避ける）
				if (id < 0 || id >= 5) continue;
				if (tileImg[id] == -1) continue;

				int drawX = startTileX + tx * TILE_W;
				int drawY = startTileY + ty * TILE_H;

				//まずタイル描画（伏せ or 開き用）
				DrawGraph(drawX, drawY, tileImg[id], TRUE);

				if (tempMap[ty][tx] == 2)
				{
					// 地雷なら爆弾画像
					if (mineMap[ty][tx] == 1)
					{
						DrawGraph(drawX, drawY, mine, TRUE);
					}
					else
					{
						int n = numMap[ty][tx];  // 0～8

						if (n == 0)
						{
							DrawGraph(drawX, drawY, tileImg[2], TRUE); // empty
						}
						else if (n >= 1 && n <= 7)
						{
							if (numImg[n] != -1) DrawGraph(drawX, drawY, numImg[n], TRUE);
						}
						// 8が出るなら保険
						// else if (n == 8) DrawFormatString(drawX + 10, drawY + 6, GetColor(0,0,0), "8");
					}
				}

				//旗の描画（伏せの上に表示）
				if (flagMap[ty][tx] == 1)
				{
					DrawGraph(drawX, drawY, gh_frag, TRUE);
				}

				// ゲームオーバーなら爆弾を全部表示
				if (gameOver && mineMap[ty][tx] == 1)
				{
					DrawGraph(drawX, drawY, mine, TRUE);
				}
			}
		}

		int aa = GetMouseInput();//入力はフレームの頭で1回だけ取得

		// 左クリックでタイルを切り替える例（伏せ→空白）
		if ((aa & MOUSE_INPUT_LEFT) && !(maemouse & MOUSE_INPUT_LEFT))
		{
			int tx = (mouseX - startTileX) / TILE_W;
			int ty = (mouseY - startTileY) / TILE_H;

			if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT)
			{
				// ★ 旗があるマスは開けない
				if (flagMap[ty][tx] == 1)
				{
					// 何もしない（開かない）
				}
				else
				{
					// 伏せ(1)だけ開ける
					if (tempMap[ty][tx] == 1)
					{

						//--------最初の左クリックで 地雷と数字を全マス生成----------------
						if (firstClick == true)
						{
							//（念のため）前のデータを消す
							for (int y = 0; y < MAP_HEIGHT; y++)
							{
								for (int x = 0; x < MAP_WIDTH; x++)
								{
									mineMap[y][x] = 0;
									numMap[y][x] = 0;
									openMap[y][x] = 0;
								}
							}

							// 地雷配置（クリック周辺3×3は安全）
							PlaceMines(tx, ty);

							// 数字計算（周囲地雷数を全マスに入れる）
							CalcNumbers();

							// 2回目以降は生成しない
							firstClick = false;
						}
						//---------------------------------------------------------------

						Reveal(tx, ty);


						// ★ここに書く！：クリア判定
						if (!gameOver)     // 地雷踏んでないときだけ
						{
							CheckClear();  // 地雷以外全部開いたら gameClear=true にする
						}


						//tempMap[ty][tx] = 2;
						//openMap[ty][tx] = 1; // 開いた記録（今後の拡張用）

						//tempMap[ty][tx] = 2;
					}
					
					if (timecount == 0)
					{
						timecount = 1;
						startTime = GetNowCount();
						TimeMs = 0; // 表示を000に固定したいなら
					}

				}
			}
		}

		// 右クリックで旗を置く（開いたマスには置けない）
		if (((aa & MOUSE_INPUT_RIGHT)) && !(maemouse & MOUSE_INPUT_RIGHT))
		{
			// マップ内のクリックならタイル座標に変換
			int tx = (mouseX - startTileX) / TILE_W;
			int ty = (mouseY - startTileY) / TILE_H;

			if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT)
			{
				// ★ 伏せマス(1)のときだけ旗を置ける
				if (tempMap[ty][tx] == 1)
				{
					// 旗をトグル（置く/消す）
					flagMap[ty][tx] = 1 - flagMap[ty][tx];

					// ★ 置いたら fr--、消したら fr++
					if (flagMap[ty][tx] == 1)
					{
						fr -= 1;   // 旗を置いた
					}
					else
					{
						fr += 1;   // 旗を消した
					}
				}
				// else: 開いたマスには何もしない
			}
		}

		//キー設定をnowRに置き換える
		int nowR = CheckHitKey(KEY_INPUT_R);

		// Rキー「押した瞬間」だけリセット（長押しで連続リセットしない）
		if (nowR && !prevR)
		{
			for (int ty = 0; ty < MAP_HEIGHT; ++ty)//縦マス
			{
				for (int tx = 0; tx < MAP_WIDTH; ++tx)//横マス
				{
					tempMap[ty][tx] = 1;   // 全部伏せに戻す
					flagMap[ty][tx] = 0;   // 旗も全部消す

					

				}
			}

			fr = 30; // 残り旗数を戻す

			timecount = 0;
			startTime = 0;
			TimeMs = 0;
		}

		prevR = nowR;

		// マップ描画：tempMap の値（= タイルID）に応じてタイルを並べる
		for (int ty = 0; ty < MAP_HEIGHT; ++ty)
		{
			for (int tx = 0; tx < MAP_WIDTH; ++tx)
			{
				int id = tempMap[ty][tx];  //←ここでtempMapを使う
				if (id == 0) continue;     //0は何も描かない扱い

				// 範囲安全策（未ロードIDを避ける）
				if (id < 0 || id >= 5) continue;
				if (tileImg[id] == -1) continue;

				int drawX = startTileX + tx * TILE_W;
				int drawY = startTileY + ty * TILE_H;

				DrawGraph(drawX, drawY, tileImg[id], TRUE);


				// 開いたマスの中身表示
				if (tempMap[ty][tx] == 2)
				{
					// 地雷なら爆弾
					if (mineMap[ty][tx] == 1)
					{
						DrawGraph(drawX, drawY, mine, TRUE);
					}
					else
					{
						int n = numMap[ty][tx];

						if (n == 0) DrawGraph(drawX, drawY, tileImg[2], TRUE);
						else if (n >= 1 && n <= 7) DrawGraph(drawX, drawY, numImg[n], TRUE);
					}
				}


				//旗の描画
				if (flagMap[ty][tx] == 1)
				{
					DrawGraph(drawX, drawY, gh_frag, TRUE);
				}
			}
		}
		
		//---経過時間処理--------------------------------------

		if (timecount == 1 && !gameOver && !gameClear)
		{
			TimeMs = GetNowCount() - startTime;
		}


		int nowTime = GetNowCount();//カウント宣言
		int TimeMs = nowTime - startTime;//カウントタイマー
		int TimeMl = TimeMs / 1000;//カウント１０００タイム

		//画像の描画(位置X、位置Y、グラフィックハンドル、透明度の有効無効)
		DrawGraph(4, 17, gh_frag, TRUE);

		SetFontSize(26);//フォントサイズ
		DrawFormatString(40, 21, GetColor(255, 255, 255), "%d", fr);//旗カウント
		DrawFormatString(90, 22, GetColor(255, 255, 255), "%03d", TimeMl);//カウントタイム表示
		SetFontSize(16); //元フォントサイズ(ほかの文字に影響出るので戻す)

		if (gameClear)
		{
			SetFontSize(40);
			DrawFormatString(150, 200, GetColor(0, 255, 0), "CLEAR");
			SetFontSize(16);
		}

		if (gameOver)
		{
			SetFontSize(40);
			DrawFormatString(120, 200, GetColor(255, 0, 0), "GAME OVER");
			SetFontSize(16);

			// 左クリックでタイルを開く
			if ((aa & MOUSE_INPUT_LEFT) && !(maemouse & MOUSE_INPUT_LEFT))
			{
				int tx = (mouseX - startTileX) / TILE_W;
				int ty = (mouseY - startTileY) / TILE_H;

				if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT)
				{
					// ★ 旗があるマスは開けない
					if (flagMap[ty][tx] == 1)
					{
						// 何もしない（開かない）
					}
					else
					{
						// 伏せ(1)だけ開ける
						if (tempMap[ty][tx] == 1)
						{
							//--------最初の左クリックで 地雷と数字を全マス生成----------------
							if (firstClick == true)
							{
								for (int y = 0; y < MAP_HEIGHT; y++)
								{
									for (int x = 0; x < MAP_WIDTH; x++)
									{
										mineMap[y][x] = 0;
										numMap[y][x] = 0;
										openMap[y][x] = 0;
									}
								}

								PlaceMines(tx, ty);
								CalcNumbers();
								firstClick = false;
							}
							//---------------------------------------------------------------

							Reveal(tx, ty);
						}

						// タイマー開始（最初の操作で開始したいならこの位置がOK）
						if (timecount == 0)
						{
							timecount = 1;
							startTime = GetNowCount();
							TimeMs = 0;
						}
					}
				}
			}

			// 右クリックで旗を置く
			if ((aa & MOUSE_INPUT_RIGHT) && !(maemouse & MOUSE_INPUT_RIGHT))
			{
				int tx = (mouseX - startTileX) / TILE_W;
				int ty = (mouseY - startTileY) / TILE_H;

				if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT)
				{
					// ★ 伏せマス(1)のときだけ旗を置ける
					if (tempMap[ty][tx] == 1)
					{
						flagMap[ty][tx] = 1 - flagMap[ty][tx];

						if (flagMap[ty][tx] == 1) fr -= 1;
						else                      fr += 1;
					}
				}
			}
		}


		if (!gameOver && !gameClear)
		{
			// 左クリックでタイルを切り替える例（伏せ→空白）
			if ((aa & MOUSE_INPUT_LEFT) && !(maemouse & MOUSE_INPUT_LEFT))
			{
				int tx = (mouseX - startTileX) / TILE_W;
				int ty = (mouseY - startTileY) / TILE_H;

				if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT)
				{
					// ★ 旗があるマスは開けない
					if (flagMap[ty][tx] == 1)
					{
						// 何もしない（開かない）
					}
					else
					{
						// 伏せ(1)だけ開ける
						if (tempMap[ty][tx] == 1)
						{

							//--------最初の左クリックで 地雷と数字を全マス生成----------------
							if (firstClick == true)
							{
								//（念のため）前のデータを消す
								for (int y = 0; y < MAP_HEIGHT; y++)
								{
									for (int x = 0; x < MAP_WIDTH; x++)
									{
										mineMap[y][x] = 0;
										numMap[y][x] = 0;
										openMap[y][x] = 0;
									}
								}

								// 地雷配置（クリック周辺3×3は安全）
								PlaceMines(tx, ty);

								// 数字計算（周囲地雷数を全マスに入れる）
								CalcNumbers();

								// 2回目以降は生成しない
								firstClick = false;
							}
							//---------------------------------------------------------------

							Reveal(tx, ty);


							// ★ここに書く！：クリア判定
							if (!gameOver)     // 地雷踏んでないときだけ
							{
								CheckClear();  // 地雷以外全部開いたら gameClear=true にする
							}


							//tempMap[ty][tx] = 2;
							//openMap[ty][tx] = 1; // 開いた記録（今後の拡張用）

							//tempMap[ty][tx] = 2;
						}

						if (timecount == 0)
						{
							timecount = 1;
							startTime = GetNowCount();
							TimeMs = 0; // 表示を000に固定したいなら
						}

					}
				}
			}

			// 右クリックで旗を置く（開いたマスには置けない）
			if (((aa & MOUSE_INPUT_RIGHT)) && !(maemouse & MOUSE_INPUT_RIGHT))
			{
				// マップ内のクリックならタイル座標に変換
				int tx = (mouseX - startTileX) / TILE_W;
				int ty = (mouseY - startTileY) / TILE_H;

				if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT)
				{
					// ★ 伏せマス(1)のときだけ旗を置ける
					if (tempMap[ty][tx] == 1)
					{
						// 旗をトグル（置く/消す）
						flagMap[ty][tx] = 1 - flagMap[ty][tx];

						// ★ 置いたら fr--、消したら fr++
						if (flagMap[ty][tx] == 1)
						{
							fr -= 1;   // 旗を置いた
						}
						else
						{
							fr += 1;   // 旗を消した
						}
					}
					// else: 開いたマスには何もしない
				}
			}
		}


		//--------------------------------------------------------------------

		ScreenFlip(); //裏画面データを表画面へ反映

		//毎ループ呼び出す。エラーになった場合breakする
		if (ProcessMessage() == -1)break;
		//エスケープキーを押したり、エラーになった場合、breakする
		if (CheckHitKey(KEY_INPUT_ESCAPE))break;

		//フレームの最後に「前の状態」を更新
		maemouse = aa;
	}

	//画像データ削除
	DeleteGraph(gh);

	WaitKey();	 //キー入力待ち
	DxLib_End(); //DXライブラリ使用の終了処理
	return 0;
};