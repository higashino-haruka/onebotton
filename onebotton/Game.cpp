#include"Game.h"
#include<time.h>

void Game::Init(HWND hWnd)
{
	srand((unsigned)time(NULL));  //rand関数の為に、srand関数が必要

	RendererInit(hWnd);  //DirectXを初期化

	//sound.Init();  //サウンドを初期化
	//sound.Play(SOUND_LABEL_BGM000);  //BGMを再生

	//画像「バックグラウンド」
	m_background.Init("asset/background.png");  //プレイヤーを初期化
	m_background.SetPos(0.0f, 0.0f, 0.0f);  //位置を設定
	m_background.SetSize(640.0f, 480.0f, 0.0f);  //大きさを設定
	m_background.SetAngle(0.0f);  //角度を設定

	for (int x = 0; x < STAGE_X; x++)
	{
		for (int y = 0; y < STAGE_Y; y++)
		{
			m_blocks[x][y].Init("asset/block.png");  //テトリミノを初期化
			m_blocks[x][y].SetPos(BLOCK_SIZE * (x - STAGE_X / 2), -BLOCK_SIZE * (y - STAGE_Y / 2), 0.0f);  //位置を設定
			m_blocks[x][y].SetSize(BLOCK_SIZE, BLOCK_SIZE, 0.0f);  //大きさを設定
		}
	}

	m_scoreboard.Init("asset/number.png", 10, 1);  //得点表示を初期化
	m_scoreboard.SetPos(-145.0f, 200.0f, 0.0f);  //位置を設定
	m_scoreboard.SetSize(40.0f, 40.0f, 0.0f);  //大きさを設定
	m_score = 0;  //得点を初期化

	m_title.Init("asset/tetorisu.png");  //タイトル表示を初期化
	m_title.SetPos(0.0f, 0.0f, 0.0f);  //位置を設定
	m_title.SetSize(640.0f, 480.0f, 0.0f);  //大きさを設定

	m_scene = 0;  //シーンを初期化

	// ホールド初期化
	m_holdType = -1;
	m_holdUsed = false;

	// ホールド枠（位置・サイズは必要に応じて調整してください）
	m_holdFrame.Init("asset/hold_frame.png");
	m_holdFrame.SetPos(-220.0f, -175.0f, 0.0f); // 例: 左上
	m_holdFrame.SetSize(100.0f, 100.0f, 0.0f);

	// NEXT枠（位置・サイズは必要に応じて調整してください）
	m_nextFrame.Init("asset/next_frame.png");
	m_nextFrame.SetPos(215.0f, 150.0f, 0.0f); // 例: 左上
	m_nextFrame.SetSize(100.0f, 200.0f, 0.0f);

	// ホールド用小ブロック初期化
	for (int hx = 0; hx < 4; hx++)
	{
		for (int hy = 0; hy < 4; hy++)
		{
			m_holdBlocks[hx][hy].Init("asset/block.png");
			m_holdBlocks[hx][hy].SetSize(20.0f, 20.0f, 0.0f);
		}
	}

	// Next初期化
	for (int i = 0; i < 5; i++) m_nextTypes[i] = -1;
	FillNextQueue();

	// プレビュー用ブロック初期化（右上に表示する想定）
	for (int n = 0; n < m_nextCount; n++)
	{
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				m_nextBlocks[n][x][y].Init("asset/block.png");
				m_nextBlocks[n][x][y].SetSize(16.0f, 16.0f, 0.0f); // 小さめ
			}
		}
	}
}

void Game::Update(void)
{
	input.Update();

	switch (m_scene)
	{
	case 0:  //タイトル画面
		//キー入力で本編へ
		if (input.GetKeyTrigger(VK_RETURN))
		{
			m_scene = 1;

			//deta配列を初期化
			for (int x = 0; x < STAGE_X; x++)
			{
				for (int y = 0; y < STAGE_Y + 3; y++)
				{
					m_data[x][y].state = 0;
					m_data[x][y].tetriminoType = -1;
				}
			}
			m_state = 0;  //ゲームの状態を初期化
			m_score = 0;  //得点を初期化
		}
		break;

	case 1:  //ゲーム本編
		//プレイヤーアニメーション

		//ゲームオーバーならゲームの進行を止める
		if (m_state == 3)
		{
			if (input.GetKeyTrigger(VK_RETURN))
			{
				m_scene = 0;
			}
			return;
		}


		//新たにテトリミノを出現させる
		// 新たにテトリミノを出現させる
		if (m_state == 0)
		{
			m_holdUsed = false; // 新しいミノが出たらホールド使用フラグをリセット
			//ゲームの状態を「１：落下中」に変更
			m_state = 1;

			// ※重要: 新しいミノ生成時に m_holdType を上書きしない
			// 先頭のキューから取り出して現在ミノにする
			m_currentType = PopNextType();
			m_currentDir = 0;  //向きをリセット

			for (int x = 0; x < 4; x++)
			{
				for (int y = 0; y < 4; y++)
				{
					m_data[x + 4][y].state = m_tetrominoData[m_currentType][m_currentDir][y][x];
					m_data[x + 4][y].tetriminoType = m_currentType; // 色分けのため追加
				}
			}

			m_count = 0;

			//落下中のテトリミノの位置をリセット
			m_currentX = 4;
			m_currentY = 0;
		}


		//テトリミノを落とす処理
		m_count++;
		if (m_count > 30)
		{
			m_count = 0;

			//揃っていれば
			if (m_state == 2)
			{
				//テトリミノ消去
				DeleteTetromino();

				//ゲームの状態を「０：落下するものがない状態」に変更
				m_state = 0;
			}

			//テトリミノ落下
			DropTetromino();
		}
		//左移動
		if (input.GetKeyTrigger(VK_LEFT))
		{
			MoveTetromino(-1);
		}
		//右移動
		if (input.GetKeyTrigger(VK_RIGHT))
		{
			MoveTetromino(1);
		}
		//回転
		if (input.GetKeyTrigger(VK_UP))
		{
			RotateTetromino();
		}

		//下移動
		if (input.GetKeyPress(VK_DOWN))
		{
			m_count = 30;
		}

		if (input.GetKeyTrigger(VK_SPACE))
		{
			HoldTetromino();
		}

		//色を付ける
		for (int x = 0; x < STAGE_X; x++)
		{
			for (int y = 0; y < STAGE_Y; y++)
			{
				switch (m_data[x][y + 3].state)
				{
				case 0:  //空白
					m_blocks[x][y].SetColor(1.0f, 1.0f, 1.0f, 0.2f);
					break;

				case 1:  //着地しているテトリミノ
					//m_blocks[x][y].SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					//break;

				case 2:  //落ちてきているテトリミノ
					//m_blocks[x][y].SetColor(1.0f, 0.8f, 0.8f, 1.0f);
					//break;
					switch (m_data[x][y + 3].tetriminoType)
					{
					case 0:  //Iミノ
						m_blocks[x][y].SetColor(0.0f, 1.0f, 1.0f, 1.0f);
						break;

					case 1:  //Jミノ
						m_blocks[x][y].SetColor(0.0f, 0.0f, 1.0f, 1.0f);
						break;

					case 2:  //Lミノ
						m_blocks[x][y].SetColor(1.0f, 0.5f, 0.0f, 1.0f);
						break;

					case 3:  //Oミノ
						m_blocks[x][y].SetColor(1.0f, 1.0f, 0.0f, 1.0f);
						break;

					case 4:  //Sミノ
						m_blocks[x][y].SetColor(0.0f, 1.0f, 0.0f, 1.0f);
						break;

					case 5:  //Tミノ
						m_blocks[x][y].SetColor(1.0f, 0.0f, 1.0f, 1.0f);
						break;

					case 6:  //Zミノ
						m_blocks[x][y].SetColor(1.0f, 0.0f, 0.0f, 1.0f);
						break;
					}
					break;

				case 3:  //揃ったテトリミノ(消去待ち)
					m_blocks[x][y].SetColor(0.4f, 0.4f, 1.0f, 1.0f);
					break;

				case 4:  //ゲームオーバー時
					m_blocks[x][y].SetColor(1.0f, 0.2f, 0.2f, 1.0f);
					break;
				}
			}
		}
		break;
	}
}

void Game::Draw(void)
{
	RendererDrawStart();  //描画開始(背面に描画したい物から)

	switch (m_scene)
	{
	case 0:  //タイトルを描画
		m_title.Draw();  //タイトルを描画
		break;

	case 1:  //ゲーム本編
		m_background.Draw();  //２枚目の「バックグラウンド」を描画

		//テトリミノを表示
		for (int x = 0; x < STAGE_X; x++)
		{
			for (int y = 0; y < STAGE_Y; y++)
			{
				m_blocks[x][y].Draw();
			}
		}

		m_holdFrame.Draw();

		if (m_holdType != -1)
		{
			const float baseX = -250.0f; // Init と揃える
			const float baseY = -200.0f;
			const float cell = 20.0f; // ブロックの表示サイズと合わせる


			for (int x = 0; x < 4; x++)
			{
				for (int y = 0; y < 4; y++)
				{
					if (m_tetrominoData[m_holdType][0][y][x] == 2)
					{
						m_holdBlocks[x][y].SetPos(baseX + x * cell, baseY + y * cell, 0.0f);


						// 色分け（既存の色付けに合わせる）
						switch (m_holdType)
						{
						case 0: m_holdBlocks[x][y].SetColor(0.0f, 1.0f, 1.0f, 1.0f); break; // I
						case 1: m_holdBlocks[x][y].SetColor(0.0f, 0.0f, 1.0f, 1.0f); break; // J
						case 2: m_holdBlocks[x][y].SetColor(1.0f, 0.5f, 0.0f, 1.0f); break; // L
						case 3: m_holdBlocks[x][y].SetColor(1.0f, 1.0f, 0.0f, 1.0f); break; // O
						case 4: m_holdBlocks[x][y].SetColor(0.0f, 1.0f, 0.0f, 1.0f); break; // S
						case 5: m_holdBlocks[x][y].SetColor(1.0f, 0.0f, 1.0f, 1.0f); break; // T
						case 6: m_holdBlocks[x][y].SetColor(1.0f, 0.0f, 0.0f, 1.0f); break; // Z
						}


						m_holdBlocks[x][y].Draw();
					}
				}
			}
		}

		// Next描画
		m_nextFrame.Draw();
		const float baseX = 200.0f; // 例: 画面右寄り
		const float baseY = 100.0f; // 例: 上寄り
		const float gapY = 48.0f;   // 各プレビューの縦間隔
		const float cell = 16.0f;

		// ↓ i = 0 が一番上に出るようにする
		for (int n = 0; n < m_nextCount; n++)
		{
			int type = m_nextTypes[n];
			if (type == -1) continue;

			// ★Y座標を「上から下」になるように調整
			float px = baseX;
			float py = baseY + (m_nextCount - 1 - n) * gapY;

			for (int x = 0; x < 4; x++)
			{
				for (int y = 0; y < 4; y++)
				{
					if (m_tetrominoData[type][0][y][x] == 2)
					{
						m_nextBlocks[n][x][y].SetPos(px + x * cell, py + y * cell, 0.0f);

						switch (type)
						{
						case 0: m_nextBlocks[n][x][y].SetColor(0.0f, 1.0f, 1.0f, 1.0f); break; // I
						case 1: m_nextBlocks[n][x][y].SetColor(0.0f, 0.0f, 1.0f, 1.0f); break; // J
						case 2: m_nextBlocks[n][x][y].SetColor(1.0f, 0.5f, 0.0f, 1.0f); break; // L
						case 3: m_nextBlocks[n][x][y].SetColor(1.0f, 1.0f, 0.0f, 1.0f); break; // O
						case 4: m_nextBlocks[n][x][y].SetColor(0.0f, 1.0f, 0.0f, 1.0f); break; // S
						case 5: m_nextBlocks[n][x][y].SetColor(1.0f, 0.0f, 1.0f, 1.0f); break; // T
						case 6: m_nextBlocks[n][x][y].SetColor(1.0f, 0.0f, 0.0f, 1.0f); break; // Z
						}
						m_nextBlocks[n][x][y].Draw();
					}
				}
			}
		}



		//スコアを表示
		DirectX::XMFLOAT3 pos = m_scoreboard.GetPos();
		DirectX::XMFLOAT3 size = m_scoreboard.GetSize();
		int keta = 0;
		do
		{
			m_scoreboard.numU = m_score % (int)pow(10, keta + 1) / (int)pow(10, keta);  //一桁を切り出す
			m_scoreboard.SetPos(pos.x - size.x * keta, pos.y, pos.z);  //位置を設定
			m_scoreboard.Draw();  //描画
			keta++;
		} while (m_score >= (int)pow(10, keta));
		m_scoreboard.SetPos(pos.x, pos.y, pos.z);  //位置を元に戻す
		break;
	}

	RendererDrawEnd();    //描画終了
}

void Game::Uninit(void)
{
	m_background.Uninit();

	//ブロックを終了
	for (int x = 0; x < STAGE_X; x++)
	{
		for (int y = 0; y < STAGE_Y; y++)
		{
			m_blocks[x][y].Uninit();
		}
	}
	m_scoreboard.Uninit();  //スコアを終了
	//sound.Uninit();  //サウンドを終了
	m_title.Uninit();  //DirectXを終了

	// ホールド表示の解放
	m_holdFrame.Uninit();
	for (int hx = 0; hx < 4; hx++)
	{
		for (int hy = 0; hy < 4; hy++)
		{
			m_holdBlocks[hx][hy].Uninit();
		}
	}

	//next
	m_nextFrame.Uninit();
	for (int n = 0; n < m_nextCount; n++)
	{
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				m_nextBlocks[n][x][y].Uninit();
			}
		}
	}

	// DirectXの解放処理
	RendererUninit();
}

void Game::HoldTetromino()
{
	// ホールドは落下中のミノに対してのみ有効（任意の制約です）


// 既にこのターンでホールドしていたら無効
	if (m_holdUsed) return;


	// まず、現在の落下ミノをステージから消す
	for (int x = 0; x < STAGE_X; x++)
	{
		for (int y = 0; y < STAGE_Y + 3; y++)
		{
			if (m_data[x][y].state == 2)
			{
				m_data[x][y].state = 0;
			}
		}
	}


	if (m_holdType == -1)
	{
		// ホールドが空 -> 現在のミノをホールドして、キューから次のミノを出す
		m_holdType = m_currentType;
		m_currentType = PopNextType();   // ← rand() ではなく Next キューから取得
	}
	else
	{
		// 入れ替え
		int tmp = m_holdType;
		m_holdType = m_currentType;
		m_currentType = tmp;
	}



	// 新しい（あるいは入れ替え後の）現在ミノの初期配置
	m_currentDir = 0;
	m_currentX = 4;
	m_currentY = 0;


	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			if (m_tetrominoData[m_currentType][m_currentDir][y][x] == 2)
			{
				m_data[m_currentX + x][m_currentY + y].state = 2;
				m_data[m_currentX + x][m_currentY + y].tetriminoType = m_currentType;
			}
		}
	}


	// 落下タイミングをリセット
	m_count = 0;


	// このターンでは再ホールド不可
	m_holdUsed = true;
}

void Game::FillNextQueue()
{
	for (int i = 0; i < 5; i++)
	{
		if (m_nextTypes[i] == -1)
		{
			m_nextTypes[i] = rand() % 7;
		}
	}
}

int Game::PopNextType()
{
	int t = m_nextTypes[0];
	// 左に詰める
	for (int i = 0; i < 4; i++)
	{
		m_nextTypes[i] = m_nextTypes[i + 1];
	}
	m_nextTypes[4] = -1; // 末尾を空にしておく
	FillNextQueue();     // 足りない分を補充
	return t;
}


void Game::DropTetromino(void)
{
	//落とすことが可能か調べる
	bool ngFg = IsInvalidMove(0, 1);

	//落とすことが不可能なら
	if (ngFg == true)
	{
		//着地
		for (int x = 0; x < STAGE_X; x++)
		{
			for (int y = 0; y < STAGE_Y + 3; y++)
			{
				if (m_data[x][y].state == 2)
				{
					m_data[x][y].state = 1;
				}
			}
		}

		//ゲームの状態を「０：落下するものがない状態」に変更
		m_state = 0;

		//１列揃った行が無いかチェックする
		for (int y = 0; y < STAGE_Y + 3; y++)
		{
			bool completeFg = true;
			for (int x = 0; x < STAGE_X; x++)
			{
				if (m_data[x][y].state == 0)
				{
					completeFg = false;
					break;
				}
			}
			//揃っていれば色を付ける
			if (completeFg == true)
			{
				for (int x = 0; x < STAGE_X; x++)
				{
					m_data[x][y].state = 3;  //消去待ち
				}

				//ゲームの状態を「２：揃った状態」に変更
				m_state = 2;
			}
		}
		//ゲームオーバーになっていないかチェックする
		CheckGameOver();
	}
	//落とすことが可能なら
	else
	{
		ShiftTetrimino(0, 1);  //移動させる
	}
}
//テトリミノ左右移動
void Game::MoveTetromino(int dirX)
{
	//落下中でなければ無視
	if (m_state != 1)
	{
		return;
	}

	//左右移動が可能か調べる
	bool ngFg = IsInvalidMove(dirX, 0);

	//左右移動が可能なら
	if (ngFg == false)
	{
		ShiftTetrimino(dirX, 0);  //移動させる
	}
}

//テトリミノ回転
void Game::RotateTetromino(void)
{
	//落下中でなければ無視
	if (m_state != 1)
	{
		return;
	}

	//向きを変更する
	int newDir = m_currentDir + 1;
	if (newDir > 3)
	{
		newDir = 0;
	}

	//回転が可能か調べる
	bool ngFg = false;
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			if (m_tetrominoData[m_currentType][newDir][y][x] == 2 && (m_data[m_currentX + x][m_currentY + y].state == 1 || m_currentX + x >= STAGE_X || m_currentX + x < 0 || m_currentY + y >= STAGE_Y + 3))
			{
				ngFg = true;
				break;
			}
		}
		if (ngFg == true)
		{
			break;
		}
	}
	//回転が可能なら
	if (ngFg == false)
	{
		// まず、回転させる落下中ミノを空白にする
		for (int x = 0; x < STAGE_X; x++)
		{
			for (int y = 0; y < STAGE_Y + 3; y++)
			{
				if (m_data[x][y].state == 2)
				{
					m_data[x][y].state = 0;
				}
			}
		}
		// 回転後のミノの形をコピーする
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				if (m_tetrominoData[m_currentType][newDir][y][x] == 2)
				{
					m_data[m_currentX + x][m_currentY + y].state = 2;
					m_data[m_currentX + x][m_currentY + y].tetriminoType = m_currentType; // 色分けのため追加
				}
			}
		}
		m_currentDir = newDir;  //方向を更新
	}
}

//テトリミノ消去
void Game::DeleteTetromino(void)
{
	//１列揃った行が無いかチェックする
	int cnt = 0;  //消した列数をカウントする変数
	for (int cy = STAGE_Y + 2; cy >= 0;)
	{
		if (m_data[0][cy].state == 3)
		{
			//消去待ちなら
			for (int y = cy - 1; y >= 0; y--)
			{
				for (int x = 0; x < STAGE_X; x++)
				{
					m_data[x][y + 1] = m_data[x][y];
				}
			}
			cnt++;
		}
		else
		{
			cy--;
		}
	}

	//得点を加算する
	switch (cnt)
	{
	case 1:  //シングル(一列消し)
		m_score += 100;
		break;

	case 2:  //ダブル(二列消し)
		m_score += 300;
		break;

	case 3:  //トリプル(三列消し)
		m_score += 500;
		break;

	case 4:  //テトリス(四列消し)
		m_score += 800;
		break;

	}
}

void Game::CheckGameOver(void)
{
	//ゲームオーバーになっていないかチェックする
	for (int tx = 0; tx < STAGE_X; tx++)
	{
		//一番上の列をチェック
		if (m_data[tx][3].state == 1)
		{
			//テトリミノの色を変える
			for (int x = 0; x < STAGE_X; x++)
			{
				for (int y = 0; y < STAGE_Y + 3; y++)
				{
					if (m_data[x][y].state == 1)
					{
						m_data[x][y].state = 4;
					}
				}
			}

			//ゲームの状態を「３：ゲームオーバー」に変更
			m_state = 3;
			// ホールドをリセット
			m_holdType = -1;
			m_holdUsed = false;
			for (int i = 0; i < 5; i++) m_nextTypes[i] = -1;
			FillNextQueue();
			break;
		}
	}
}

//テトリミノを移動が無効かどうか確認
bool Game::IsInvalidMove(int dirX, int dirY)
{
	//落とすことが可能か調べる
	bool ngFg = false;
	for (int x = 0; x < STAGE_X; x++)
	{
		for (int y = 0; y < STAGE_Y + 3; y++)
		{
			if (m_data[x][y].state == 2 && (x + dirX < 0 || x + dirX >= STAGE_X || y + dirY >= STAGE_Y + 3 || m_data[x + dirX][y + dirY].state == 1))
			{	//                     ↑左端    　↑右端       　　　　↑下端　　　　　　↑すでにテトリミノがある
				ngFg = true;
				break;
			}
		}
		if (ngFg == true)
		{
			break;
		}
	}
	return ngFg;
}

//m_dataを更新してテトリミノを移動させる
void Game::ShiftTetrimino(int dirX, int dirY)
{
	BLOCK_INFO tmp[STAGE_X][STAGE_Y + 3] = {};
	for (int x = 0; x < STAGE_X; x++)
	{
		for (int y = 0; y < STAGE_Y + 3; y++)
		{
			if (m_data[x][y].state == 2)
			{
				tmp[x + dirX][y + dirY] = m_data[x][y]; // 色分けのため変更;
			}
			else if (m_data[x][y].state == 1)
			{
				tmp[x][y] = m_data[x][y];
			}
		}
	}
	for (int x = 0; x < STAGE_X; x++)
	{
		for (int y = 0; y < STAGE_Y + 3; y++)
		{
			m_data[x][y] = tmp[x][y];
		}
	}

	//落下中のテトリミノの位置を更新
	m_currentX += dirX;
	m_currentY += dirY;
}

// 画面遷移修正のため追加
// 画面遷移を適用する
void Game::ApplySceneTransition()
{
	if (m_next_scene != -1) // 画面遷移が必要な場合
	{
		m_scene = m_next_scene; // 現在シーンを変更
		m_next_scene = -1; // 画面遷移なしにリセット
	}
}
