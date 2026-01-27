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

	// 読み込み失敗チェック（任意）
	if (gh == -1 || gh_frag == -1 || tileImg[1] == -1) {
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

	// マウス座標取得（& と <= は C++ 記法で）
	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	int maemouse = 0;

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

		int aa = GetMouseInput();//入力はフレームの頭で1回だけ取得

		// 左クリックでタイルを切り替える例（伏せ→空白）
		if (((aa & MOUSE_INPUT_LEFT)) && !(maemouse & MOUSE_INPUT_LEFT))
		{

			// マップ内のクリックならタイル座標に変換
			int tx = (mouseX - startTileX) / TILE_W;
			int ty = (mouseY - startTileY) / TILE_H;

			if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT) 
			{
				if (tempMap[ty][tx] == 1)
				{
					//1～２に変更
					tempMap[ty][tx] = 2; 
				}
			}
		}

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
			}
		}

		DrawGraph(0, 50, tileImg[1], TRUE);
		//画像の描画(位置X、位置Y、グラフィックハンドル、透明度の有効無効)

		DrawGraph(3, 17, gh_frag, TRUE);;

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
}