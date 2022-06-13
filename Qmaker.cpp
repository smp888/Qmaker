#include <mpi.h>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <ctime>

using namespace std;

int IndexOf(string str, string sub) 
{
	if (str.find(sub)== std::string::npos)
	{
		return -1;
	}
	else
	{
		return str.find(sub);
	}
}

vector<string> Split(string str, char delim)
{
    vector<string> result;
	string token = "";
	for (auto i = 0; i <= str.size(); i++)
	{
		if ((str[i] == delim)||(i == str.size()))
		{
			if (token!="")
			{
				result.push_back(token);
			}
			token="";
		}
		else
		{
			token=token+str[i];
		}
	}
    return result;
}

vector <int> inparams; //параметры программы

int iterations = 0; //количество итераций
map <string, int> paramrazm; //параметры размерности
vector<string> indata; //входные переменные

map <string, string> outdata; //выходные переменные

int numberinway = 0; 
map <int, string> way;

struct Vertice //Структура блока блок-схемы
{                  
    int Id;
    int Type;
	string Content;
}; 
struct Edge //Структура соединения блок-схемы
{                  
    int From;
	int To;
    int Type;
}; 
vector<Vertice> Vertices; //Блоки блок-схемы
vector<Edge> Edges; //Соединения блок-схемы

void ReadFile() //Чтение блок-схемы из файла
{
	string path = "BS.txt";
	ifstream Fstr;
	Fstr.open(path, ios::in);
	string one;
	string countiesJson = "";
	while (!Fstr.eof())
	{
		getline(Fstr,one);
		countiesJson = countiesJson + one;
	}
	Fstr.close();
	string token = "";
	string last_token = "";
	bool is_edges = false;
	for (auto i = 0; i < countiesJson.size(); i++)
	{
		if ((countiesJson[i] == '"')||(i== countiesJson.size()-1))
		{
			if (is_edges)
			{
				if (last_token == "From")
				{
					Edge e;
					Edges.push_back(e);
					Edges[Edges.size() - 1].From = stoi(token.substr(1, token.size() - 2));
				}
				if (last_token == "To")
				{
					Edges[Edges.size() - 1].To = stoi(token.substr(1, token.size() - 2));
				}
				if (last_token == "Type")
				{
					Edges[Edges.size() - 1].Type = stoi(token.substr(1, 1));
				}
			}
			else
			{
				if (last_token == "Id")
				{
					Vertice e;
					Vertices.push_back(e);
					Vertices[Vertices.size() - 1].Id = stoi(token.substr(1,token.size()-2));
				}
				if (last_token == "Type")
				{
					Vertices[Vertices.size() - 1].Type = stoi(token.substr(1, token.size() - 2));
				}
				if (last_token == ":")
				{
					Vertices[Vertices.size() - 1].Content = token;
				}
				if (token == "Edges")
				{
					is_edges = true;
				}
			}
			last_token = token;
			token = "";
		}
		else
		{
			token = token + countiesJson[i];
		}
	}
}

vector<string> countie2;
vector<string> countie3;
vector<string> countie4;

void GetDim()//извлечение из блок-схемы параметров размерности
{
	for (auto ver : Vertices)
    {
		if (ver.Type == 4)
		{
			countie2.push_back(ver.Content);
		}
    }
	for (auto inperem : countie2)
	{
		int indexOfCharOpen = IndexOf(inperem,"[");
		int indexOfCharClose = IndexOf(inperem,"]");
		if (indexOfCharOpen >= 0)
		{
			string param = inperem.substr(indexOfCharOpen + 1, indexOfCharClose - indexOfCharOpen - 1);
			if (IndexOf(param,",") < 0)//блок-схема имеет один параметр размерности
			{
				bool b = false;
				for (auto c : paramrazm)
				{
					if (c.first == param) { b = true; };
				}
				if (b == false)
				{
					int paramValue;
					paramValue = inparams.back();
					inparams.pop_back();
					paramrazm[param]=paramValue;//Сохранение параметра размерности и его значения в словаре.
				}
			}
			if (IndexOf(param,",") > 0)//блок-схема имеет несколько параметров размерности
			{
				vector<string> masparam = Split(param,',');
				for (auto s : masparam)
				{
					bool b = false;
					for (auto c : paramrazm)
					{
						if (c.first == s) { b = true; };
					}
					if (b == false)
					{
						int sValue;
						sValue = inparams.back();
						inparams.pop_back();
						paramrazm[s]=sValue;//Сохранение параметра размерности и его значения в словаре.
					}
				}
			}
		}
	}
}

void GetInVal()//извлечение из блок-схемы входных переменных
{
	for (auto ver : Vertices)
    {
		if (ver.Type == 4)
		{
			countie3.push_back(ver.Content);
		}
    }
	for (auto inper : countie3)
	{
		int indexOfCharOpen = IndexOf(inper,"[");
		int indexOfCharClose = IndexOf(inper,"]");
		if (indexOfCharOpen < 0) indata.push_back(inper);
		if ((indexOfCharOpen > 0)&(IndexOf(inper,",") < 0))
		{
			for (auto c : paramrazm)
			{
				if (c.first == inper.substr(indexOfCharOpen + 1, indexOfCharClose - indexOfCharOpen - 1))
				{
					for (int r = 1; r <= c.second; r++)
					{
						string inp = inper.substr(0, indexOfCharOpen) + "(" + to_string(r) + ")";
						indata.push_back(inp);
					}
				}
			}
		}
		if ((indexOfCharOpen > 0) & (IndexOf(inper,",") > 0))
		{
			string outper = inper.substr(0, indexOfCharOpen);//Обозначение  входной переменной.
			string param = inper.substr(indexOfCharOpen + 1, indexOfCharClose - indexOfCharOpen - 1);
			vector<string> masparam = Split(param,',');
			int i = 0;
			int indexoutdata[10];
			for (auto s : masparam)
			{
				for (auto c : paramrazm)
				{
					if (c.first == s)
					{
						indexoutdata[i] = c.second;
						i++;
					}
				}
			}
			i--;
			int indexout[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };//вспомогательный массив для перебора значений массива indexoutdata
			string qterm;
			bool endpereb;
			do
			{
				qterm = outper;
				string indexqterm = "(" + to_string(indexout[0]);
				for (int j = 1; j <= i; j++)
				{
					indexqterm = indexqterm + "," + to_string(indexout[j]);
				}
				qterm = qterm + indexqterm + ")";
				indata.push_back(qterm);

				endpereb = true;
				for (int r = 0; r <= i; r++)
				{
					endpereb = endpereb & (indexout[r] == indexoutdata[r]);//условие окончания
				}
				if (endpereb == true) break;//Все наборы индексов для выходной переменной обработаны
				for (int k = i; k >= 0; k--)
				{
					if (indexout[k] < indexoutdata[k])
					{
						indexout[k]++;
						for (int r = k + 1; r < 10; r++) indexout[r] = 1;
						break;
					}
				}
			}
			while (endpereb == false);
		}
		for (auto o : indata)
		{
			if (o == "iterations")
			{
				iterations = inparams.back();
				inparams.pop_back();
			}
		}
	}
}

void GetOutVal()//извлечение из блок-схемы выходных переменных
{
	for (auto ver : Vertices)
	{
		if (ver.Type == 5)
		{
			countie4.push_back(ver.Content);
		}
	}
	for (auto inperem : countie4)
	{
		int indexOfCharOpen = IndexOf(inperem,"[");
		int indexOfCharClose = IndexOf(inperem,"]");
		if (indexOfCharOpen < 0) outdata[inperem]="0";//Сохранение выходной переменной и ее значения в словаре outdata, когда алгоритм не имеет размерности
		if ((indexOfCharOpen > 0) & (IndexOf(inperem,",") < 0))
		{
			for (auto c : paramrazm)
			{
				if (c.first == inperem.substr(indexOfCharOpen + 1, indexOfCharClose - indexOfCharOpen - 1))
				{
					for (int r = 1; r <= c.second; r++)
					{
						string inp = inperem.substr(0, indexOfCharOpen) + "(" + to_string(r) + ")";
						outdata[inp]="0";
					}
				}
			}
		}
		if ((indexOfCharOpen > 0) & (IndexOf(inperem,",") > 0))
		{
			string outper = inperem.substr(0, indexOfCharOpen);//Обозначение выходной переменной.
			string param = inperem.substr(indexOfCharOpen + 1, indexOfCharClose - indexOfCharOpen - 1);
			vector<string> masparam = Split(param, ',');
			int i = 0;
			int indexoutdata[10];
			for (auto s : masparam)
			{
				for (auto c : paramrazm)
				{
					if (c.first == s)
					{
						indexoutdata[i] = c.second;
						i++;
					}
				}
			}
			i--;
			int indexout[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };//вспомогательный массив для перебора значений массива indexoutdata
			string qterm;
			bool endpereb;
			do
			{
				qterm = outper;
				string indexqterm = "(" + to_string(indexout[0]);
				for (int j = 1; j <= i; j++)
				{
					indexqterm = indexqterm + "," + to_string(indexout[j]);
				}
				qterm = qterm + indexqterm + ")";
				outdata[qterm]="0";

				endpereb = true;
				for (int r = 0; r <= i; r++)
				{
					endpereb = endpereb & (indexout[r] == indexoutdata[r]);//условие окончания
				}
				if (endpereb == true) break;//Все наборы индексов для выходной переменной обработаны
				for (int k = i; k >= 0; k--)
				{
					if (indexout[k] < indexoutdata[k])
					{
						indexout[k]++;
						for (int r = k + 1; r < 10; r++) indexout[r] = 1;
						break;
					}
				}
			}
			while (endpereb == false);
		}
	}
}

void Skip(int argc, int r, int k)
{
	if (argc == 2)
	{
		for (int i=0;i<r-1;i++)
		{
			if (way.size() <= numberinway)
			{
				numberinway = numberinway + 1;
				way[numberinway] = to_string(0) + "," + "1";
			}
			if (numberinway < way.size())
			{
				numberinway = numberinway + 1;
			}
		}
	}
	if (argc == 3)
	{
		int kk = k;
		if ((kk>iterations) && (iterations>0))
		{
			kk = iterations;
		}
		if ((kk > 2) && (argc == 1))
		{
			kk = 2;
		}
		for (int i = 0; i < kk; i++)
		{
			if (way.size() <= numberinway)
			{
				numberinway = numberinway + 1;
				way[numberinway] = to_string(0) + "," + "1";
			}
			if (numberinway < way.size())
			{
				numberinway = numberinway + 1;
			}
		}
	}
	if (argc == 4)
	{
		if (way.size() <= numberinway)
		{
			numberinway = numberinway + 1;
			way[numberinway] = to_string(0) + "," + "1";
		}
		if (numberinway < way.size())
		{
			numberinway = numberinway + 1;
		}
	}
}

void PassBS()//Проход по блок-схеме от начала до конца по всем путям
{
	map <string, string> vnutrperem;
	vnutrperem["empout"] = "1";
	string logicalqterm = " ";
	int maxnumberbl = 0;//хранит количество блоков в блок-схеме
	int nextb = 0;//инициализация переменной для хранения номера текущего блока
	for (auto ver : Vertices)
	{
		if (ver.Type == 0)
		{
			nextb = ver.Id;
		}
		if (ver.Id > maxnumberbl) { maxnumberbl = ver.Id; }
	}
	while (nextb < maxnumberbl)
	{ 
	for (auto ver : Vertices)
	{
		if (ver.Id == nextb)
		{
				if (ver.Type == 2)
				{
					string content = ver.Content;
					int assignmentcharacter = IndexOf(content,"=");
					string leftcontent = content.substr(0, assignmentcharacter);
					if (IndexOf(leftcontent,"[") < 0)
					{
						if (IndexOf(leftcontent,"(") > 0) //НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРЕМЕННОЙ СЛЕВА ОТ ЗНАКА ПРИСВАИВАНИЯ, если она иммет индексы
						{
							int leftbracket = IndexOf(leftcontent,"(");
							string leftcontentname = leftcontent.substr(0, leftbracket);//имя переменной слева от скобки (
							string leftotprisv = leftcontentname + "(";
							string vnutriskobok = leftcontent.substr(IndexOf(leftcontent,"(") + 1, IndexOf(leftcontent,")") - IndexOf(leftcontent,"(") - 1);
							if (IndexOf(leftcontent,",") > 0)//несколько индексов
							{
								vector<string> masindex = Split(vnutriskobok, ',' );
								for (string s : masindex)
								{
									for (auto c : vnutrperem)//ищутся внутренние переменные для индексов переменной слева от знака присваивания и определяются их значения
									{
										if (c.first == s)
										{
											leftotprisv = leftotprisv + vnutrperem[s] + ",";
										}
									}
								}
								leftotprisv = leftotprisv.substr(0, leftotprisv.size() - 1) + ")";
							}
							if (IndexOf(leftcontent,",") < 0)//один индекс
							{
								for (auto c : vnutrperem)//ищется внутренняя переменная для индекса переменной слева от знака присваивания и определяется ее значение
								{
									if (c.first == vnutriskobok)
									{
										leftotprisv = leftotprisv + vnutrperem[vnutriskobok];
									}
								}
								leftotprisv = leftotprisv.substr(0, leftotprisv.size()) + ")";                                            
							}
							leftcontent = leftotprisv;
							//КОНЕЦ ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРЕМЕННОЙ СЛЕВА ОТ ЗНАКА ПРИСВАИВАНИЯ, ЕСЛИ 2 И БОЛЕЕ ИНДЕКСОВ, а также 1 индекс
						}
						bool outp = false;
						for (auto c : outdata)//Проверяется, находится ли слева от знака присваивания выходная переменная.
						{
							if (c.first == leftcontent) { outp = true; };
						}
						if (outp == true) //слева от знака присваивания выходная переменная
						{
							string rightcontent = content.substr(assignmentcharacter + 1, content.size() - assignmentcharacter - 1);
							if ((IndexOf(rightcontent,"+") < 0) & (IndexOf(rightcontent,"-") < 0) & (IndexOf(rightcontent,"*") < 0) & (IndexOf(rightcontent,"/") < 0))
							{
								if ((rightcontent.substr(0, 1) == "0") || (rightcontent.substr(0, 1) == "1") || (rightcontent.substr(0, 1) == "2") || (rightcontent.substr(0, 1) == "3") || (rightcontent.substr(0, 1) == "4") || (rightcontent.substr(0, 1) == "5") || (rightcontent.substr(0, 1) == "6") ||
								   (rightcontent.substr(0, 1) == "7") || (rightcontent.substr(0, 1) == "8") || (rightcontent.substr(0, 1) == "9"))
								{
									outdata[leftcontent] = rightcontent;
								}
								bool rightcontentinp = false;
								for (auto c : indata)// Проверяется, справа от присваивания входная переменная?
								{
									if (c == rightcontent) { rightcontentinp = true; };
								}
								if (rightcontentinp == true)
								{
									outdata[leftcontent] = rightcontent;
								}
								if ((IndexOf(rightcontent,"(") > 0) & (IndexOf(rightcontent,",") < 0))
								{
									string rightc = rightcontent.substr(IndexOf(rightcontent,"(") + 1, IndexOf(rightcontent,")") - IndexOf(rightcontent,"(") - 1);
									if ((rightc.substr(0, 1) == "0") || (rightc.substr(0, 1) == "1") || (rightc.substr(0, 1) == "2") || (rightc.substr(0, 1) == "3") || (rightc.substr(0, 1) == "4") || (rightc.substr(0, 1) == "5") || (rightc.substr(0, 1) == "6") ||
								   (rightc.substr(0, 1) == "7") || (rightc.substr(0, 1) == "8") || (rightc.substr(0, 1) == "9"))
									{
										for (auto c : vnutrperem)
										{
											//if (c == vnutrperem[rightcontent])
											if (c.first == rightcontent)
											{
												//System.Console.WriteLine($"Проверяется, что {rightcontent} является внутренней переменной");
												//System.Console.ReadKey();
												outdata[leftcontent] = vnutrperem[rightcontent];
												//System.Console.WriteLine($"В выходную переменную {leftcontent} записано значение {rightcontent.substr(0, rightcontentIndexOf(,"(")) + "(" + vnutrperem[rightc] + ")"}");
												//System.Console.WriteLine($"В выходную переменную {leftcontent} записано значение {vnutrperem[rightcontent]}");
												//System.Console.ReadKey();
											}
										}                                                    
									}
									else
									{
										for (auto c : indata)
										{
											if (c == (rightcontent.substr(0, IndexOf(rightcontent,"(")) + "(" + vnutrperem[rightc] + ")"))
											{
												outdata[leftcontent] = rightcontent.substr(0, IndexOf(rightcontent,"(")) + "(" + vnutrperem[rightc] + ")";
											}
										}
										for (auto c : vnutrperem)
										{
											if (c.first == (rightcontent.substr(0, IndexOf(rightcontent,"(")) + "(" + vnutrperem[rightc] + ")"))
											{
												outdata[leftcontent] = vnutrperem[rightcontent.substr(0, IndexOf(rightcontent,"(")) + "(" + vnutrperem[rightc] + ")"];
											}
										}
									}
									
								}
								if ((IndexOf(rightcontent,"(") > 0) & (IndexOf(rightcontent,",") > 0))//справа переменная с нсколькими индексами
								{
									int leftbracket = IndexOf(rightcontent,"(");
									string rightcontentname = rightcontent.substr(0, leftbracket);//имя переменной справа от знака присваивания
									//System.Console.WriteLine($"Имя переменной справа от знака присваивания {rightcontentname}");
									//System.Console.ReadKey();
									//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРЕМЕННОЙ СПРАВА ОТ ЗНАКА ПРИСВАИВАНИЯ
									string rightcontentv = rightcontentname + "(";
									string vnutriskobok = rightcontent.substr(IndexOf(rightcontent,"(") + 1, IndexOf(rightcontent,")") - IndexOf(rightcontent,"(") - 1);
									vector<string> masindex = Split(vnutriskobok, ',');
									for (string s : masindex)
									{
										//System.Console.WriteLine($"{s} - индекс переменной справа от знака присваивания.");
										//System.Console.ReadKey();
										for (auto c : vnutrperem)//ищутся внутренние переменные для индексов переменной справа от знака присваивания и определяются их значения
										{
											if (c.first == s)
											{
												rightcontentv = rightcontentv + vnutrperem[s] + ",";
												//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} переменной справа от знака присваивания.");
												//System.Console.ReadKey();
											}
										}
										for (auto c : paramrazm)//ищутся внутренние переменные для индексов переменной справа от знака присваивания и определяются их значения
										{
											if (c.first == s)
											{
												rightcontentv = rightcontentv + to_string(paramrazm[s]) + ",";
												//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} переменной справа от знака присваивания.");
												//System.Console.ReadKey();
											}
										}
									}
									rightcontentv = rightcontentv.substr(0, rightcontentv.size() - 1) + ")";
									//System.Console.WriteLine($"{rightcontentv} - значение справа от знака присваивания.");
									//System.Console.ReadKey();
									bool rcv = false;
									for (auto c : vnutrperem)//определяется, является ли значение справа от знака присваивания внутренней переменной
									{
										if (c.first == rightcontentv)
										{
											rcv = true;
										}
									}
									if (rcv == true)
									{
										outdata[leftcontent] = vnutrperem[rightcontentv];//изменено значение выходной переменной
									}
									//System.Console.WriteLine($"Выходная переменная {leftcontent} изменена, ее значением является {vnutrperem[rightcontentv]}");
									//System.Console.ReadKey();
									//09.12.2021 НАЧАЛО ВСТАВКИ: определяется, является ли значение справа от знака присваивания входной переменной с несколькими индексами
									bool rcinp = false;
									for (auto c : indata)//определяется, является ли значение справа от знака присваивания входной переменной
									{
										if (c == rightcontentv)
										{
											rcinp = true;
										}
									}
									if (rcinp == true)
									{
										outdata[leftcontent] = rightcontentv;//изменено значение выходной переменной
									}
								}
							}
							if ((IndexOf(rightcontent,"+") > 0))
							{
								int locationoperation = IndexOf(rightcontent,"+");
								string leftrightcontent = rightcontent.substr(0, locationoperation);
								//System.Console.WriteLine($"Слева от операции сложения {leftrightcontent}");
								//System.Console.ReadKey();
								string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
								//System.Console.WriteLine($"Справа от операции сложения {rightrightcontent}");
								//System.Console.ReadKey();
								if ((IndexOf(leftrightcontent,"(") > 0))
								{
									int leftbracket = IndexOf(leftrightcontent,"(");
									string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции сложения
									//System.Console.WriteLine($"Имя переменной слева от операции сложения {leftrightcontentname}");
									//System.Console.ReadKey();
									//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРВОГО СЛАГАЕМОГО - теперь нужно проверить, является ли оно выходной переменной
									string leftsomnoj = leftrightcontentname + "(";
									string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);
									vector<string> masindex = Split(vnutriskobok, ',');
									for (string s : masindex)
									{
										//System.Console.WriteLine($"{s} - индекс левого слагаемого.");//теперь нужно получить значение индексов левого слагаемого
										//System.Console.ReadKey();
										for (auto c : vnutrperem)//ищутся внутренние переменные для индексов первого слагаемого и определяются их значения
										{
											if (c.first == s)
											{
												leftsomnoj = leftsomnoj + vnutrperem[s] + ",";
												//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} левого слагаемого.");
												//System.Console.ReadKey();
											}
										}
									}
									leftsomnoj = leftsomnoj.substr(0, leftsomnoj.size() - 1) + ")";
									//System.Console.WriteLine($"{leftsomnoj} - значение левого слагаемого.");
									//System.Console.ReadKey();
									//Далее нужно внести исправления для определения, является ли левое слагаемое выходной переменной
									if (outdata[leftsomnoj] != "0")//Избавляемся от сложения с "0" при вычислении Cij
									{
										//string leftvalue = "{" + "\"op\":\"+\",\"fO\":" + $"{outdata[leftsomnoj]},";
										//03.12.2021 НАЧАЛО: ВСТАВКА ИСПОЛЬЗУЕТСЯ ВМЕСТО ПРЕДЫДУЩЕЙ СТРОКИ ДЛЯ УЧЕТА СЛУЧАЯ, КОГДА ПЕРВОЕ СЛАГАЕМОЕ НЕ ИМЕЕТ СКОБОК { и }
										string leftvalue = " ";
										if ((IndexOf(outdata[leftsomnoj],"{") >= 0))
										{
											leftvalue = "{\"op\":\"+\",\"fO\":" + outdata[leftsomnoj]+ ",";
										}
										else
										{
											leftvalue = "{\"op\":\"+\",\"fO\":\"" + outdata[leftsomnoj]+"\",";
										}
										outdata[leftcontent] = leftvalue;//изменение значения выходной переменной
									}                                                
								}
								if ((IndexOf(leftrightcontent,"(") < 0))
								{
									//System.Console.WriteLine($"Имя переменной слева от операции сложения {leftrightcontent}");
									//System.Console.ReadKey();                                                
									bool leftrightcontentoutp = false;
									for (auto c : outdata)// Проверяется, находится ли слева от операции сложения выходная переменная.
									{
										if (c.first == leftrightcontent)
										{ leftrightcontentoutp = true; };
									}
									if (leftrightcontentoutp == true)
									{                                                                                                         

											if ((IndexOf(outdata[leftrightcontent],"{") >= 0))
											{
												outdata[leftcontent] = "{\"op\":\"+\",\"fO\":" + outdata[leftrightcontent]+",";
										}
											else
											{
												outdata[leftcontent] = "{\"op\":\"+\",\"fO\":\"" + outdata[leftrightcontent]+"\",";
										}                                                                                                                                                               
									}
								}
								if ((IndexOf(rightrightcontent,"(") < 0))
								{
									bool rightrightcontentvp = false;
									for (auto c : vnutrperem)// Проверяется, находится ли справа от операции сложения внутренняя переменная.
									{
										if (c.first == rightrightcontent)
										{ rightrightcontentvp = true; };
									}
									if (rightrightcontentvp == true)
									{
										if (outdata[leftcontent].substr(0, 1) == "{")//Избавляемся от сложения с "0" при вычислении Cij
									{
										//outdata[leftcontent] = outdata[leftcontent] + $"{vnutrperem[rightrightcontent]})";//изменение значения выходной переменной

										if ((IndexOf(vnutrperem[rightrightcontent],"{") >= 0))
										{
											outdata[leftcontent] = outdata[leftcontent] + "\"sO\":"+vnutrperem[rightrightcontent]+ "}";
										}
										else
										{
											outdata[leftcontent] = outdata[leftcontent] + "\"sO\":\""+vnutrperem[rightrightcontent]+"\"}";
										}
									}
									else
									{
										outdata[leftcontent] = vnutrperem[rightrightcontent];//изменение значения выходной переменной
									}                                    
									}
								}
								if ((IndexOf(rightrightcontent,"(") > 0))
								{
									int leftbracketr = IndexOf(rightrightcontent,"(");
									string rightrightcontentname = rightrightcontent.substr(0, leftbracketr);//имя переменной слева от скобки (
									//System.Console.WriteLine($"Имя второго слагаемого {rightrightcontentname}");
									//System.Console.ReadKey();
									string vtoroeslagaemoe = rightrightcontentname + "(";
									string vnutriskobokr = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
									if (IndexOf(rightrightcontent,",") > 0)//несколько индексов
									{
										vector<string> masindex = Split(vnutriskobokr, ',' );
										for (string s : masindex)
										{
										//System.Console.WriteLine($"{s} - индекс второго слагаемого.");//теперь нужно получить значение индексов второго слагаемого
										//System.Console.ReadKey();
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов второго слагаемого и определяются их значения
											{
												if (c.first == s)
												{
													vtoroeslagaemoe = vtoroeslagaemoe + vnutrperem[s] + ",";
													//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} второго слагаемого.");
													//System.Console.ReadKey();
												}
											}
										}
									vtoroeslagaemoe = vtoroeslagaemoe.substr(0, vtoroeslagaemoe.size() - 1) + ")";
									//System.Console.WriteLine($"{vtoroeslagaemoe} - значение второго слагаемого с несколькими индексами.");
									//System.Console.ReadKey();
									}
									bool rightrightcontentinp = false;
									for (string c : indata)// Проверяется, является ли второе слагаемое входной переменной.
									{
										if (c == vtoroeslagaemoe) { rightrightcontentinp = true; };
									}
									if (rightrightcontentinp == true)
									{                                                    
										if (outdata[leftcontent].substr(0, 1) == "{")
										{                                                        
											outdata[leftcontent] = outdata[leftcontent] + "\"sO\":\""+vtoroeslagaemoe+"\"}";
											//System.Console.WriteLine($"Новым значением выходной переменной {leftcontent} является {outdata[leftcontent]}");
											//System.Console.ReadKey();
										}
										else
										{
											outdata[leftcontent] = vtoroeslagaemoe;
											//System.Console.WriteLine($"Новым значением выходной переменной {leftcontent} является {vtoroeslagaemoe}");
											//System.Console.ReadKey();
										}
									}
								}   
							}
							if ((IndexOf(rightcontent,"-") > 0))
							{
								int locationoperation = IndexOf(rightcontent,"-");
								string leftrightcontent = rightcontent.substr(0, locationoperation);
								//System.Console.WriteLine($"Слева от операции вычитания {leftrightcontent}");
								//System.Console.ReadKey();
								string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
								//System.Console.WriteLine($"Справа от операции вычитания {rightrightcontent}");
								//System.Console.ReadKey();
								string leftrightcontentvalue = " ";
								if ((IndexOf(leftrightcontent,"(") < 0))//уменьшаемое
								{                                                     
									if ((leftrightcontent.substr(0, 1) == "0") || (leftrightcontent.substr(0, 1) == "1") || (leftrightcontent.substr(0, 1) == "2") || (leftrightcontent.substr(0, 1) == "3") || (leftrightcontent.substr(0, 1) == "4") || (leftrightcontent.substr(0, 1) == "5") || (leftrightcontent.substr(0, 1) == "6") ||
									   (leftrightcontent.substr(0, 1) == "7") || (leftrightcontent.substr(0, 1) == "8") || (leftrightcontent.substr(0, 1) == "9"))
									{
										//System.Console.WriteLine($"Значение уменьшаемого является числом {leftrightcontent}");
										//System.Console.ReadKey();
										leftrightcontentvalue = leftrightcontent;
									}                                               
								}
								if ((IndexOf(rightrightcontent,"(") < 0))//вычитаемое
								{                                                
									bool rightrightcontentvp = false;
									for (auto c : vnutrperem)// Проверяется, находится ли справа от операции вычитания внутренняя переменная.
									{
										if (c.first == rightrightcontent)
										{ rightrightcontentvp = true; };
									}
									if (rightrightcontentvp == true)
									{
										if ((IndexOf(vnutrperem[rightrightcontent],"{")) >= 0)
										{
											outdata[leftcontent] = "{\"op\":\"-\",\"fO\":\"" + leftrightcontentvalue + "\",\"sO\":" + vnutrperem[rightrightcontent] + "}";
										}
										else
										{
											outdata[leftcontent] = "{\"op\":\"-\",\"fO\":\"" + leftrightcontentvalue + "\",\"sO\":\"" + vnutrperem[rightrightcontent]+"\"}";
										}
									}                                                
								}
							}
							if ((IndexOf(rightcontent,"-") == 0))//операция унарный минус
							{
								int locationoperation = IndexOf(rightcontent,"-");
								string leftrightcontent = rightcontent.substr(0, locationoperation);
								string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);                                        
								if ((IndexOf(rightrightcontent,"(") < 0))//вычитаемое не содержит индексов
								{
									bool rightrightcontentvp = false;
									for (auto c : vnutrperem)// Проверяется, находится ли справа от операции унарного минуса внутренняя переменная.
									{
										if (c.first == rightrightcontent)
										{ rightrightcontentvp = true; };
									}
									if (rightrightcontentvp == true)
									{
										if ((IndexOf(vnutrperem[rightrightcontent],"{")) >= 0)
										{
											outdata[leftcontent] = "{\"op\":\"-\",\"od\":" + vnutrperem[rightrightcontent] + "}";
										}
										else
										{
											outdata[leftcontent] = "{\"op\":\"-\",\"od\":\"" + vnutrperem[rightrightcontent]+"\"}";
										}
										//System.Console.WriteLine($"В выходную переменную {leftcontent} зпаписано значение {outdata[leftcontent]}");
										//System.Console.ReadKey();
									}
								}
							}
							if ((IndexOf(rightcontent,"*") > 0))
							{
								int locationoperation = IndexOf(rightcontent,"*");
								string leftrightcontent = rightcontent.substr(0, locationoperation);
								//System.Console.WriteLine($"Слева от операции умножения {leftrightcontent}");
								//System.Console.ReadKey();
								string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
								//System.Console.WriteLine($"Справа от операции умножения {rightrightcontent}");
								//System.Console.ReadKey();
								if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") < 0))
								{
									int leftbracket = IndexOf(leftrightcontent,"(");
									string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции умножения
									//System.Console.WriteLine($"Имя переменной слева от операции умножения {leftrightcontentname}");
									//System.Console.ReadKey();
									//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРВОГО СОМНОЖИТЕЛЯ
									string leftsomnoj = leftrightcontentname + "(";
									string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);
									//System.Console.WriteLine($"Внутри скобок первого сомножителя {vnutriskobok}");
									//System.Console.ReadKey();
									bool vnutriskobokvp = false;
									for (auto c : vnutrperem)//проверяется, является ли индексом первого сомножителя внутренняя переменная
									{
										if (c.first == vnutriskobok)
										{
											vnutriskobokvp = true;
										}
									}
									if (vnutriskobokvp == true)
									{
										leftsomnoj = leftsomnoj + vnutrperem[vnutriskobok] + ")";
										//System.Console.WriteLine($"{leftsomnoj} - значение первого сомножителя.");
										//System.Console.ReadKey();
										outdata[leftcontent] = "{\"op\":\"*\",\"fO\":\"" + leftsomnoj+"\",";                                                    
										//System.Console.WriteLine($"Значение выходной переменной {leftcontent} изменено в коллекции, ее значением является {outdata[leftcontent]}");
										//System.Console.ReadKey();
									}                                      
								}
								//КОНЕЦ ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРВОГО СОМНОЖИТЕЛЯ
								//ВТОРОЙ СОМНОЖИТЕЛЬ
								if ((IndexOf(rightrightcontent,"(") > 0) & (IndexOf(rightrightcontent,",") < 0))
								{
									int rightbracket = IndexOf(rightrightcontent,"(");
									string rightrightcontentname = rightrightcontent.substr(0, rightbracket);//Имя переменной спраа от операции умножения
									//System.Console.WriteLine($"Имя переменной справа от операции умножения {rightrightcontentname}");
									//System.Console.ReadKey();
									//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРВОГО СОМНОЖИТЕЛЯ
									string rightsomnoj = rightrightcontentname + "(";
									string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
									//System.Console.WriteLine($"Внутри скобок второго сомножителя {vnutriskobok}");
									//System.Console.ReadKey();
									bool vnutriskobokvp = false;
									for (auto c : vnutrperem)//проверяется, является ли индексом первого сомножителя внутренняя переменная
									{
										if (c.first == vnutriskobok)
										{
											vnutriskobokvp = true;
										}
									}
									if (vnutriskobokvp == true)
									{
										rightsomnoj = rightsomnoj + vnutrperem[vnutriskobok] + ")";
										//System.Console.WriteLine($"{rightsomnoj} - значение правого сомножителя.");
										//System.Console.ReadKey();
										outdata[leftcontent] = outdata[leftcontent] + "\"sO\":\""+rightsomnoj+"\"}";
										//System.Console.WriteLine($"Значение выходной переменной {leftcontent} изменено в коллекции, ее значением является {outdata[leftcontent]}");
										//System.Console.ReadKey();
									}
								}
								if ((IndexOf(rightrightcontent,"(") < 0))
								{
									//System.Console.WriteLine($"Имя переменной справа от операции сложения {rightrightcontent}");
									//System.Console.ReadKey();
									if (outdata[leftcontent].substr(0, 1) == "{")//Избавляемся от сложения с "0" при вычислении Cij
									{
										//outdata[leftcontent] = outdata[leftcontent] + $"{vnutrperem[rightrightcontent]})";//изменение значения выходной переменной
										outdata[leftcontent] = outdata[leftcontent] + "\"sO\":"+vnutrperem[rightrightcontent] + "}";
										//System.Console.WriteLine($"Новым значением выходной переменной {leftcontent} является {outdata[leftcontent]}");
										//System.Console.ReadKey();
									}
									else
									{
										outdata[leftcontent] = vnutrperem[rightrightcontent];//изменение значения выходной переменной
										//System.Console.WriteLine($"Новым значением выходной переменной {leftcontent} является {outdata[leftcontent]}");
										//System.Console.ReadKey();
									}
								}
							}
							if ((IndexOf(rightcontent,"/") > 0))
							{
								int locationoperation = IndexOf(rightcontent,"/");
								string leftrightcontent = rightcontent.substr(0, locationoperation);
								//System.Console.WriteLine($"Слева от операции деления {leftrightcontent}");
								//System.Console.ReadKey();
								string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
								//System.Console.WriteLine($"Справа от операции деления {rightrightcontent}");
								//System.Console.ReadKey();                                            
								if ((IndexOf(leftrightcontent,"(") < 0))//делимое
								{
									bool leftrightcontentvp = false;
									for (auto c : vnutrperem)// Проверяется, находится ли слева от операции деления внутренняя переменная.
									{
										if (c.first == leftrightcontent)
										{ leftrightcontentvp = true; };
									}
									if (leftrightcontentvp == true)
									{
										if ((IndexOf(vnutrperem[leftrightcontent],"{")) >= 0)
										{
											outdata[leftcontent] = "{\"op\":\"/\",\"fO\":" + vnutrperem[leftrightcontent]+",";
										}
										else
										{
											outdata[leftcontent] = "{\"op\":\"/\",\"fO\":\"" + vnutrperem[leftrightcontent]+"\",";
										}                                                    
										//System.Console.WriteLine($"Во входную переменную {leftcontent} записано сначала значение {outdata[leftcontent]}");
										//System.Console.ReadKey();
									}
								}
								if ((IndexOf(rightrightcontent,"(") < 0))//делитель
								{
									//System.Console.WriteLine($"Имя переменной справа от операции деления {rightrightcontent}");
									//System.Console.ReadKey();
									bool rightrightcontentvp = false;
									for (auto c : vnutrperem)// Проверяется, находится ли справа от операции деления внутренняя переменная.
									{
										if (c.first == rightrightcontent)
										{ rightrightcontentvp = true; };
									}
									if (rightrightcontentvp == true)
									{
										if ((IndexOf(vnutrperem[rightrightcontent],"{")) >= 0)
										{
											outdata[leftcontent] = outdata[leftcontent] + "\"sO\":" + vnutrperem[rightrightcontent] + "}";
										}
										else
										{
											outdata[leftcontent] = outdata[leftcontent] + "\"sO\":\"" + vnutrperem[rightrightcontent]+"\"}";
										}
										//System.Console.WriteLine($"Во входную переменную {leftcontent} записано окончательное значение {outdata[leftcontent]}");
										//System.Console.ReadKey();
									}
								}
							}
						}
						if (outp == false) //слева от знака присваивания не выходная переменная
						{
							//System.Console.WriteLine("Слева не выходная переменная");
							//System.Console.ReadKey();
							bool vnutp = false;
							for (auto c : vnutrperem)// Проверяется, находится ли слева от знака присваивания внутренняя переменная.
							{
								if (c.first == leftcontent) { vnutp = true; };
							}
							if (vnutp == true) //слева от знака присваивания внутренняя переменная
							{
								//System.Console.WriteLine($"Слева от знака присваивания внутренняя переменная {leftcontent}");
								//System.Console.ReadKey();
								string rightcontent = content.substr(assignmentcharacter + 1, content.size() - assignmentcharacter - 1);
								//System.Console.WriteLine($"Справа от знака присваивания {rightcontent}");
								//System.Console.ReadKey();
								if ((IndexOf(rightcontent,"+") < 0) & (IndexOf(rightcontent,"-") < 0) & (IndexOf(rightcontent,"*") < 0) & (IndexOf(rightcontent,"/") < 0) & (IndexOf(rightcontent,"abs") < 0) & (IndexOf(rightcontent,"sqrt") < 0))
								{
									if ((rightcontent.substr(0, 1) == "0") || (rightcontent.substr(0, 1) == "1") || (rightcontent.substr(0, 1) == "2") || (rightcontent.substr(0, 1) == "3") || (rightcontent.substr(0, 1) == "4") || (rightcontent.substr(0, 1) == "5") || (rightcontent.substr(0, 1) == "6") ||
									   (rightcontent.substr(0, 1) == "7") || (rightcontent.substr(0, 1) == "8") || (rightcontent.substr(0, 1) == "9"))
									{
										//System.Console.WriteLine($"Значение справа от знака присваивания является числом {rightcontent}");
										//System.Console.ReadKey();
										vnutrperem[leftcontent] = rightcontent;
										//System.Console.WriteLine($"Значение внутренней переменной {leftcontent} заменено на {rightcontent}");
										//System.Console.ReadKey();
									}
									if (IndexOf(rightcontent,"(") < 0) 
									{
										bool vnpr = false;
										for (auto c : vnutrperem)//определяется, является ли справа от знака присваивания переменная без индексов внутренней переменной
										{
											if (c.first == rightcontent)
											{
												vnpr = true;
												//System.Console.WriteLine($"Справа от присваивания внутренняя переменная {rightcontent}.");
												//System.Console.ReadKey();
											}
										}
										if (vnpr == true)
										{
											vnutrperem[leftcontent] = vnutrperem[rightcontent];
											//System.Console.WriteLine($"Во внутреннюю переменную {leftcontent} записано значение {vnutrperem[rightcontent]}.");
											//System.Console.ReadKey();
										}
									}
										if ((IndexOf(rightcontent,"(") > 0) & (IndexOf(rightcontent,",") < 0))//один индекс
									{
										int rightbracket = IndexOf(rightcontent,"(");
										string rightcontentname = rightcontent.substr(0, rightbracket);//имя переменной слева от скобки (
										string rightotprisv = rightcontentname + "(";
										string vnutriskobok = rightcontent.substr(IndexOf(rightcontent,"(") + 1, IndexOf(rightcontent,")") - IndexOf(rightcontent,"(") - 1);
										for (auto c : vnutrperem)//ищется внутренняя переменная для индекса переменной справа от знака присваивания и определяется ее значение
										{
											if (c.first == vnutriskobok)
											{
												rightotprisv = rightotprisv + vnutrperem[vnutriskobok];
												//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} переменной справа от знака присваивания.");
												//System.Console.ReadKey();
											}
										}
										rightotprisv = rightotprisv.substr(0, rightotprisv.size()) + ")";
										rightcontent = rightotprisv;
										bool vnpr = false;
										for (auto c : vnutrperem)//определяется, является ли справа от знака присваивания переменная с одним индексом внутренней переменной
										{
											if (c.first == rightotprisv)
											{
												vnpr = true;
												//System.Console.WriteLine($"Справа от присваивания внутренняя переменная {rightotprisv}.");
												//System.Console.ReadKey();
											}
										}
										bool outpr = false;
										for (auto c : outdata)//определяется, является ли справа от знака присваивания переменная с одним индексом выходной переменной
										{
											if (c.first == rightotprisv)
											{
												outpr = true;
												//System.Console.WriteLine($"Слева от присваивания внутренняя переменная {leftcontent}, а справа от присваивания выходная переменная {rightotprisv}.");
												//System.Console.ReadKey();
											}
										}
										string rightotprisvv = " ";
										if (vnpr == true)
										{
											rightotprisvv = vnutrperem[rightotprisv];
										}
										if (outpr == true)
										{
											rightotprisvv = outdata[rightotprisv];
											//System.Console.WriteLine($"Слева от присваивания внутренняя переменная {leftcontent}, ее значение {vnutrperem[leftcontent]}.");
											//System.Console.WriteLine($"Справа от присваивания выходная переменная {rightotprisv}, ее значение {rightotprisvv}.");
											//System.Console.ReadKey();
										}
										if ((rightotprisvv.substr(0, 1) == "0") || (rightotprisvv.substr(0, 1) == "1") || (rightotprisvv.substr(0, 1) == "2") || (rightotprisvv.substr(0, 1) == "3") || (rightotprisvv.substr(0, 1) == "4") || (rightotprisvv.substr(0, 1) == "5") || (rightotprisvv.substr(0, 1) == "6") ||
										   (rightotprisvv.substr(0, 1) == "7") || (rightotprisvv.substr(0, 1) == "8") || (rightotprisvv.substr(0, 1) == "9"))
										{
											rightcontent = rightotprisvv;
										}
										else
										{                                                    
												rightcontent = rightotprisv;
										}
										vnutrperem[leftcontent] = rightcontent;
									}
									if ((IndexOf(rightcontent,"(") > 0) & (IndexOf(rightcontent,",") > 0))//справа переменная с нсколькими индексами
									{
										int leftbracket = IndexOf(rightcontent,"(");
										string rightcontentname = rightcontent.substr(0, leftbracket);//имя переменной справа от знака присваивания
										//System.Console.WriteLine($"Имя переменной справа от знака присваивания {rightcontentname}");
										//System.Console.ReadKey();
										//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРЕМЕННОЙ СПРАВА ОТ ЗНАКА ПРИСВАИВАНИЯ
										string rightcontentv = rightcontentname + "(";
										string vnutriskobok = rightcontent.substr(IndexOf(rightcontent,"(") + 1, IndexOf(rightcontent,")") - IndexOf(rightcontent,"(") - 1);
										vector<string> masindex = Split(vnutriskobok, ',');
										for (string s : masindex)
										{
											//System.Console.WriteLine($"{s} - индекс переменной справа от знака присваивания.");
											//System.Console.ReadKey();
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов переменной справа от знака присваивания и определяются их значения
											{
												if (c.first == s)
												{
													rightcontentv = rightcontentv + vnutrperem[s] + ",";
													//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} переменной справа от знака присваивания.");
													//System.Console.ReadKey();
												}
											}
										}
										rightcontentv = rightcontentv.substr(0, rightcontentv.size() - 1) + ")";
										//System.Console.WriteLine($"{rightcontentv} - значение справа от знака присваивания.");
										//System.Console.ReadKey();
										//10.12.2021 НАЧАЛО ВСТАВКИ: определяется, является ли значение справа от знака присваивания входной переменной
										bool rcinp = false;
										for (string c : indata)//определяется, является ли значение справа от знака присваивания входной переменной
										{
											if (c == rightcontentv)
											{
												rcinp = true;
											}
										}
										if (rcinp == true)
										{
											vnutrperem[leftcontent] = rightcontentv;
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, в нее записана входная переменная со значением {rightcontentv}");                                                                                                               
											//System.Console.ReadKey();
										}
										//10.12.2021 КОНЕЦ ВСТАВКИ: определяется, является ли значение справа от знака присваивания входной переменной
										bool rcv = false;
										for (auto c : vnutrperem)//определяется, является ли значение справа от знака присваивания внутренней переменной
										{
											if (c.first == rightcontentv)
											{
												rcv = true;
											}
										}
										if (rcv == true)
										{
											vnutrperem[leftcontent] = vnutrperem[rightcontentv];
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, в нее записано значение {vnutrperem[rightcontentv]}");
											//System.Console.ReadKey();
										}
										else
										{
											vnutrperem[leftcontent] = rightcontentv;
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, в нее записано значение {rightcontentv}");
											//System.Console.ReadKey();
										}
									}
								}
								if ((IndexOf(rightcontent,"+") > 0))
								{
									int locationoperation = IndexOf(rightcontent,"+");
									string leftrightcontent = rightcontent.substr(0, locationoperation);
									//System.Console.WriteLine($"Слева от операции сложения {leftrightcontent}");
									//System.Console.ReadKey();
									string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
									//System.Console.WriteLine($"Справа от операции сложения {rightrightcontent}");
									//System.Console.ReadKey();
									string leftrightcontentvalue = "";
									string rightrightcontentvalue = "";
									bool leftrightcontentvp = false;
									bool rightrightcontentvp = false;
									if ((IndexOf(leftrightcontent,"(") < 0))
									{                                                    
										for (auto c : vnutrperem)// Проверяется, находится ли слева от операции сложения внутренняя переменная.
										{
											if (c.first == leftrightcontent) { leftrightcontentvp = true; };
										}
										if (leftrightcontentvp == true)
										{
											leftrightcontentvalue = vnutrperem[leftrightcontent];
											//System.Console.WriteLine($"Слева от операции сложения {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										//КОНЕЦ ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРВОГО СЛАГАЕМОГО
									}
									//Парсер второго слагаемого

									if ((IndexOf(rightrightcontent,"(") < 0))
									{
										//System.Console.WriteLine($"Имя переменной справа от операции сложения {rightrightcontent}");
										//System.Console.ReadKey();
										if ((rightrightcontent.substr(0, 1) == "0") || (rightrightcontent.substr(0, 1) == "1") || (rightrightcontent.substr(0, 1) == "2") || (rightrightcontent.substr(0, 1) == "3") || (rightrightcontent.substr(0, 1) == "4") || (rightrightcontent.substr(0, 1) == "5") || (rightrightcontent.substr(0, 1) == "6") ||
										   (rightrightcontent.substr(0, 1) == "7") || (rightrightcontent.substr(0, 1) == "8") || (rightrightcontent.substr(0, 1) == "9"))
										{
											//System.Console.WriteLine($"Значение второго слагаемого является числом {rightrightcontent}");
											//System.Console.ReadKey();
											string rightvalue = to_string((stoi(leftrightcontentvalue)) + (stoi(rightrightcontent)));
											vnutrperem[leftcontent] = rightvalue;
											//System.Console.WriteLine($"Значение внутренней переменной {leftcontent} обновлено и составляет {rightvalue}");
											//System.Console.ReadKey();
										}
										for (auto c : vnutrperem)// Проверяется, находится ли справа от операции сложения внутренняя переменная.
										{
											if (c.first == rightrightcontent) { rightrightcontentvp = true; };
										}
										if (rightrightcontentvp == true)
										{
											rightrightcontentvalue = vnutrperem[rightrightcontent];
											//System.Console.WriteLine($"Справа от операции сложения {rightrightcontent}, ее значение {rightrightcontentvalue}");
											//System.Console.ReadKey();                                                        
										}
										if ((leftrightcontentvp == true) & (rightrightcontentvp == true))
										{
											if ((leftrightcontentvalue.substr(0, 1) == "0") || (leftrightcontentvalue.substr(0, 1) == "1") || (leftrightcontentvalue.substr(0, 1) == "2") || (leftrightcontentvalue.substr(0, 1) == "3") || (leftrightcontentvalue.substr(0, 1) == "4") || (leftrightcontentvalue.substr(0, 1) == "5") || (leftrightcontentvalue.substr(0, 1) == "6") ||
											(leftrightcontentvalue.substr(0, 1) == "7") || (leftrightcontentvalue.substr(0, 1) == "8") || (leftrightcontentvalue.substr(0, 1) == "9"))
											{
												if ((rightrightcontentvalue.substr(0, 1) == "0") || (rightrightcontentvalue.substr(0, 1) == "1") || (rightrightcontentvalue.substr(0, 1) == "2") || (rightrightcontentvalue.substr(0, 1) == "3") || (rightrightcontentvalue.substr(0, 1) == "4") || (rightrightcontentvalue.substr(0, 1) == "5") || (rightrightcontentvalue.substr(0, 1) == "6") ||
												(rightrightcontentvalue.substr(0, 1) == "7") || (rightrightcontentvalue.substr(0, 1) == "8") || (rightrightcontentvalue.substr(0, 1) == "9"))
												{
													string rightvalue = to_string((stoi(leftrightcontentvalue)) + (stoi(rightrightcontentvalue)));
													vnutrperem[leftcontent] = rightvalue;
													//System.Console.WriteLine($"Внутренняя переменная p равна {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
											}
											else
											{
												//11.12.2021 НАЧАЛО ЗАМЕНЫ: Учитываем, содержат ли leftrightcontentvalue и rightrightcontentvalue скобки { и }
												if ((IndexOf(leftrightcontentvalue,"{") < 0) & (IndexOf(rightrightcontentvalue,"{") < 0))
												{
													vnutrperem[leftcontent] = "{\"op\":\"+\",\"fO\":\"" + leftrightcontentvalue+"\"," + "\"sO\":\"" + rightrightcontentvalue+"\"}";
												}
												if ((IndexOf(leftrightcontentvalue,"{") < 0) & (IndexOf(rightrightcontentvalue,"{") >= 0))
												{
													vnutrperem[leftcontent] = "{\"op\":\"+\",\"fO\":\"" + leftrightcontentvalue+"\"," + "\"sO\":" + rightrightcontentvalue + "}";
												}
												if ((IndexOf(leftrightcontentvalue,"{") >= 0) & (IndexOf(rightrightcontentvalue,"{") < 0))
												{
													vnutrperem[leftcontent] = "{\"op\":\"+\",\"fO\":" + leftrightcontentvalue+"," + "\"sO\":\"" + rightrightcontentvalue+"\"}";
												}
												if ((IndexOf(leftrightcontentvalue,"{") >= 0) & (IndexOf(rightrightcontentvalue,"{") >= 0))
												{
													vnutrperem[leftcontent] = "{\"op\":\"+\",\"fO\":" + leftrightcontentvalue+"," + "\"sO\":" + rightrightcontentvalue + "}";
												}
												//11.12.2021 КОНЕЦ ЗАМЕНЫ: Учитываем, содержат ли leftrightcontentvalue и rightrightcontentvalue скобки { и }                                                            
											//НАЧАЛО: ВЫЯВЛЕНИЕ ПРИЧИНЫ ИСКЛЮЧЕНИЯ
											//vnutrperem[leftcontent] = "{" + "\"op\": \"+\", \"fO\": ";
											//vnutrperem[leftcontent] = vnutrperem[leftcontent] + $"{leftrightcontentvalue},";
											//System.Console.WriteLine($"Значением leftrightcontentvalue является {leftrightcontentvalue}");
											//System.Console.ReadKey();
											//vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":" + $"{ rightrightcontentvalue}" + "}";
											//КОНЕЦ: ВЫЯВЛЕНИЕ ПРИЧИНЫ ИСКЛЮЧЕНИЯ
											//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
											//System.Console.ReadKey();
											}
										}
									}
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") < 0))
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции сложения
										//System.Console.WriteLine($"Имя переменной слева от операции сложения {leftrightcontentname}");
										//System.Console.ReadKey();
										string leftslag = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);

										leftslag = leftslag + vnutrperem[vnutriskobok] + ")";
										//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} первого слагаемого.");
										//System.Console.ReadKey();

										bool leftslagvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли первое слагаемое внутренней переменной.
										{
											if (c.first == leftslag) { leftslagvp = true; };
										}
										if (leftslagvp == true)
										{
											leftrightcontentvalue = vnutrperem[leftslag];
											//System.Console.WriteLine($"Слева от операции сложения {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										string leftvalue = " ";
										if ((IndexOf(leftrightcontentvalue,"{") == 0))
										{
											leftvalue = "{\"op\":\"+\",\"fO\":" + leftrightcontentvalue+",";
											vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
											//System.Console.WriteLine("В leftrightcontentvalue есть {");
											//System.Console.ReadKey();
										}
										else
										{
											leftvalue = "{\"op\":\"+\",\"fO\":\"" + leftrightcontentvalue+"\",";
											vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
										}
										//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {leftvalue}");
										//System.Console.ReadKey();
									}
									//КОНЕЦ ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРВОГО СЛАГАЕМОГО
									//Парсер второго слагаемого                                               
									if ((IndexOf(rightrightcontent,"(") > 0) & (IndexOf(rightrightcontent,",") < 0))
									{
										int leftbracket = IndexOf(rightrightcontent,"(");
										string rightrightcontentname = rightrightcontent.substr(0, leftbracket);//Имя переменной справа от операции сложения
										//System.Console.WriteLine($"Имя переменной справа от операции сложения {rightrightcontentname}");
										//System.Console.ReadKey();
										string rightslag = rightrightcontentname + "(";
										string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);

										rightslag = rightslag + vnutrperem[vnutriskobok] + ")";
										//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} второго слагаемого.");
										//System.Console.ReadKey();

										bool rightslagvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли второе слагаемое внутренней переменной.
										{
											if (c.first == rightslag) { rightslagvp = true; };
										}
										if (rightslagvp == true)
										{
											rightrightcontentvalue = vnutrperem[rightslag];
											//System.Console.WriteLine($"Справа от операции сложения {rightrightcontent}, ее значение {rightrightcontentvalue}");
											//System.Console.ReadKey();
											vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightrightcontentvalue + "}";
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {vnutrperem[leftcontent]}");
											//System.Console.ReadKey();
										}                                                    
									}
									//11.12.2021 НАЧАЛО ВСТАВКИ: слева от операции сложения внутренняя переменная с несколькими индексами, а справа без индексов
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") > 0))
									{
										int leftrightbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftrightbracket);//имя переменной слева от операции сложения
										//System.Console.WriteLine($"Имя переменной слева от операции сложения {leftrightcontentname}");
										//System.Console.ReadKey();
										//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРЕМЕННОЙ СЛЕВА ОТ ОПЕРАЦИИ СЛОЖЕНИЯ
										string leftrightcontentvnp = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);
										vector<string> masindex = Split(vnutriskobok, ',');
										for (string s : masindex)
										{
											//System.Console.WriteLine($"{s} - индекс переменной слева от операции сложения.");
											//System.Console.ReadKey();
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов переменной слева от операции сложения и определяются их значения
											{
												if (c.first == s)
												{
													leftrightcontentvnp = leftrightcontentvnp + vnutrperem[s] + ",";
													//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} переменной слева от операции сложения.");
													//System.Console.ReadKey();
												}
											}
										}
										leftrightcontentvnp = leftrightcontentvnp.substr(0, leftrightcontentvnp.size() - 1) + ")";
										//System.Console.WriteLine($"{leftrightcontentvnp} - значение переменной слева от операции сложения.");
										//System.Console.ReadKey();                                                   
										bool rcv = false;
										for (auto c : vnutrperem)//определяется, является ли значение справа от знака присваивания внутренней переменной
										{
											if (c.first == leftrightcontentvnp)
											{
												rcv = true;
											}
										}
										if (rcv == true)
										{
											leftrightcontentvalue = vnutrperem[leftrightcontentvnp];
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, в нее записано значение {vnutrperem[rightcontentv]}");
											//System.Console.ReadKey();
										}
										string leftvalue = " ";
										if ((IndexOf(leftrightcontentvalue,"{") == 0))
										{
											leftvalue = "{\"op\":\"+\",\"fO\":" + leftrightcontentvalue+",";
											vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
											//System.Console.WriteLine("В leftrightcontentvalue есть {");
											//System.Console.ReadKey();
										}
										else
										{
											leftvalue = "{\"op\":\"+\",\"fO\":\"" + leftrightcontentvalue+"\",";
											vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
										}
										//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {leftvalue}");
										//System.Console.ReadKey();
										if (rightrightcontentvp == true)
										{
											rightrightcontentvalue = vnutrperem[rightrightcontent];
											//System.Console.WriteLine($"Справа от операции сложения {rightrightcontent}, ее значение {rightrightcontentvalue}");
											//System.Console.ReadKey();
											vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightrightcontentvalue+ "}";
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {vnutrperem[leftcontent]}");
											//System.Console.ReadKey();
										}
									}
								}
								if ((IndexOf(rightcontent,"-") > 0))
								{
									int locationoperation = IndexOf(rightcontent,"-");
									string leftrightcontent = rightcontent.substr(0, locationoperation);
									//System.Console.WriteLine($"Слева от операции вычитания {leftrightcontent}");
									//System.Console.ReadKey();
									string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
									//System.Console.WriteLine($"Справа от операции вычитания {rightrightcontent}");
									//System.Console.ReadKey();
									string leftrightcontentvalue = "";
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") < 0))
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции вычитания
										//System.Console.WriteLine($"Имя переменной слева от операции вычитания {leftrightcontentname}");
										//System.Console.ReadKey();
										string leftsomnoj = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);

										leftsomnoj = leftsomnoj + vnutrperem[vnutriskobok] + ")";
										//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} уменьшаемого.");
										//System.Console.ReadKey();

										bool leftrightcontentvpnew = false;
										for (auto c : vnutrperem)// Проверяется, является ли уменьшаемое внутренней переменной.
										{
											if (c.first == leftsomnoj) { leftrightcontentvpnew = true; };
										}
										if (leftrightcontentvpnew == true)
										{
											leftrightcontentvalue = vnutrperem[leftsomnoj];
											//System.Console.WriteLine($"Слева от операции вычитания {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										bool leftrightcontentop = false;
										for (auto c : outdata)// Проверяется, является ли уменьшаемое выходной переменной.
										{
											if (c.first == leftsomnoj) { leftrightcontentop = true; };
										}
										if (leftrightcontentop == true)
										{
											leftrightcontentvalue = outdata[leftsomnoj];
											//System.Console.WriteLine($"Слева от операции вычитания {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										if ((IndexOf(leftrightcontentvalue,"{") == 0))
										{
											string leftvalue = "{\"op\":\"-\",\"fO\":" + leftrightcontentvalue+",";
											vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
											//System.Console.WriteLine("В leftrightcontentvalue есть {");
											//System.Console.ReadKey();
										}
										else
										{
											string leftvalue = "{\"op\":\"-\",\"fO\":\"" + leftrightcontentvalue+"\",";
											vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
										}
									}
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") > 0)) //Левый сомножитель с несколькими индексами
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции вычитания
										//System.Console.WriteLine($"Имя переменной слева от операции вычитания {leftrightcontentname}");
										//System.Console.ReadKey();
										string leftsomnoj = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);

										vector<string> masindex = Split(vnutriskobok, ',' );
										for (string s : masindex)
										{
											//System.Console.WriteLine($"{s} - индекс уменьшаемого.");//теперь нужно получить значение индексов уменьшаемого
											//System.Console.ReadKey();
											for (auto c : vnutrperem)//ищем внутренние переменные для индексов уменьшаемого и определяются их значения
											{
												if (c.first == s)
												{
													leftsomnoj = leftsomnoj + vnutrperem[s] + ",";
													//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} уменьшаемого.");
													//System.Console.ReadKey();
												}
											}
										}
										leftsomnoj = leftsomnoj.substr(0, leftsomnoj.size() - 1) + ")";
										//System.Console.WriteLine($"{leftsomnoj} - значение уменьшаемого после определения индексов.");
										//System.Console.ReadKey();
										bool leftsomnojvnutp = false;
										for (auto c : vnutrperem)//определяем, является ли уменьшаемое внутренней переменной.
										{
											if (c.first == leftsomnoj)
											{
												leftsomnojvnutp = true;
											}
										}
										string leftvalue = " ";
										if (leftsomnojvnutp == true)
										{
											if (IndexOf(vnutrperem[leftsomnoj],"{") < 0)
											{
												leftvalue = "{\"op\":\"-\",\"fO\":\"" + vnutrperem[leftsomnoj]+"\",";
												//System.Console.WriteLine($"Значением 1 (не JSON ) является {leftvalue}");
												//System.Console.ReadKey();
											}
											if (IndexOf(vnutrperem[leftsomnoj],"{") >= 0)
											{
												leftvalue = "{\"op\":\"-\",\"fO\":" + vnutrperem[leftsomnoj]+",";
												//System.Console.WriteLine($"Значением 2 (JSON) является {leftvalue}");
												//System.Console.ReadKey();
											}
										}
										if (leftsomnojvnutp == false)
										{
											//System.Console.WriteLine($"Левый сомножитель {leftsomnoj} не является внутренней переменной");
											//System.Console.ReadKey();
											leftvalue = "{\"op\":\"-\",\"fO\":\"" + leftsomnoj+"\",";
										}
										vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
									bool leftrightcontentvp = false;
									string rightvaluenew = " ";
									for (auto c : vnutrperem)// Проверяется, находится ли слева от операции вычитания внутренняя переменная.
									{
										if (c.first == leftrightcontent)
										{ leftrightcontentvp = true; };
									}
									bool leftrightcontentinp= false;
									if (leftrightcontentvp == true)
									{
										for (string c : indata)// Проверяется, находится ли слева от операции вычитания внутренняя переменная,содержащая входную переменную
										{
											if ((IndexOf(vnutrperem[leftrightcontent],c)) >= 0)
											{ leftrightcontentinp = true; };
										}
										if (leftrightcontentinp == true)
										{
											//System.Console.WriteLine($"значение внутренней переменной слева от операции вычитания {leftrightcontent} содержит входные переменные");
											//System.Console.ReadKey();
										}
									}
									//13.12.2021 КОНЕЦ ВСТАВКИ: Проверяем, содержит ли уменьшаемое входную переменную
									//13.12.2021 НАЧАЛО ЗАМЕНЫ: Учитываем случаи, когда значение уменьшаемого содержит входные переменные и не содержит их
									if ((leftrightcontentvp == true) & (leftrightcontentinp == true))
									{
										if ((IndexOf(vnutrperem[leftrightcontent],"{")) >= 0)
										{
											rightvaluenew = "{\"op\":\"-\",\"fO\":" + vnutrperem[leftrightcontent]+",";
										}
										else
										{
											rightvaluenew = "{\"op\":\"-\",\"fO\":\"" + vnutrperem[leftrightcontent]+"\",";
										}                                                    
										//System.Console.WriteLine($"Значение внутренней переменной {leftcontent}");
										vnutrperem[leftcontent] = rightvaluenew;
										//System.Console.WriteLine($"стало значением {rightvaluenew}");
										//System.Console.ReadKey();
									}
									if ((leftrightcontentvp == true) & (leftrightcontentinp == false))
									{
										leftrightcontentvalue = vnutrperem[leftrightcontent];
										//System.Console.WriteLine($"Значением внутренней переменной {leftrightcontent} является {leftrightcontentvalue}");                                              
										//System.Console.ReadKey();
									}
									if ((IndexOf(rightrightcontent,"(") < 0))
									{
										//System.Console.WriteLine($"Имя переменной справа от операции вычитания {rightrightcontent}");
										//System.Console.ReadKey();
										if ((rightrightcontent.substr(0, 1) == "0") || (rightrightcontent.substr(0, 1) == "1") || (rightrightcontent.substr(0, 1) == "2") || (rightrightcontent.substr(0, 1) == "3") || (rightrightcontent.substr(0, 1) == "4") || (rightrightcontent.substr(0, 1) == "5") || (rightrightcontent.substr(0, 1) == "6") ||
										   (rightrightcontent.substr(0, 1) == "7") || (rightrightcontent.substr(0, 1) == "8") || (rightrightcontent.substr(0, 1) == "9"))
										{
											//System.Console.WriteLine($"Вычитаемое является числом {rightrightcontent}");
											//System.Console.ReadKey();
											//13.12.2021 НАЧАЛО ЗАМЕНА: операция + заменена на - ;
											//string rightvalue = ((stoi(leftrightcontentvalue)) + (stoi(rightrightcontent))).ToString();
											string rightvalue = to_string((stoi(leftrightcontentvalue)) - (stoi(rightrightcontent)));
											//13.12.2021 КОНЕЦ ЗАМЕНА: операция + заменена на - ;
											vnutrperem[leftcontent] = rightvalue;
											//System.Console.WriteLine($"Значение внутренней переменной {leftcontent} обновлено и составляет {rightvalue}");
											//System.Console.ReadKey();

										}
										bool rightrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли вычитаемое внутренней переменной.
										{
											if (c.first == rightrightcontent) { rightrightcontentvp = true; };
										}                                                   
											string rightrightcontentvalue = " ";
											if (rightrightcontentvp == true)
											{
												if ((IndexOf(vnutrperem[leftcontent],"{")) >= 0)
												{
													rightrightcontentvalue = vnutrperem[rightrightcontent];
													if ((IndexOf(rightrightcontentvalue,"{")) >= 0)
													{
														vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightrightcontentvalue + "}";
														//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
														//System.Console.ReadKey();
													}
													else
													{
														vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+rightrightcontentvalue+"\"}";
														//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
														//System.Console.ReadKey();
													}
												}
											}
										}
									if ((IndexOf(rightrightcontent,"(") > 0) & (IndexOf(rightrightcontent,",") < 0))
									{
										int leftbracket = IndexOf(rightrightcontent,"(");
										string rightrightcontentname = rightrightcontent.substr(0, leftbracket);//Имя переменной справа от операции вычитания
										//System.Console.WriteLine($"Имя переменной справа от операции вычитания {rightrightcontentname}");
										//System.Console.ReadKey();
										string rightsomnoj = rightrightcontentname + "(";
										string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);

										rightsomnoj = rightsomnoj + vnutrperem[vnutriskobok] + ")";
										//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} вычитаемого.");
										//System.Console.ReadKey();

										bool rightrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли вычитаемое внутренней переменной.
										{
											if (c.first == rightsomnoj) { rightrightcontentvp = true; };
										}
										if (rightrightcontentvp == true)
										{
											string rightrightcontentvalue = vnutrperem[rightsomnoj];
											//System.Console.WriteLine($"Справа от операции вычитания {rightrightcontent}, ее значение {rightrightcontentvalue}");
											//System.Console.ReadKey();
											vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightrightcontentvalue+ "}";
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {vnutrperem[leftcontent]}");
											//System.Console.ReadKey();
										}
										bool rightrightcontentop = false;
										for (auto c : outdata)// Проверяется, является ли вычитаемое выходной переменной.
										{
											if (c.first == rightsomnoj) { rightrightcontentop = true; };
										}
										if (rightrightcontentop == true)
										{
											string rightrightcontentvalue = outdata[rightsomnoj];
											//System.Console.WriteLine($"Справа от операции вычитания {rightrightcontent}, ее значение {rightrightcontentvalue}");
											//System.Console.ReadKey();
											vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightrightcontentvalue + "}";
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {vnutrperem[leftcontent]}");
											//System.Console.ReadKey();
										}
									}
									//15.12.2021 НАЧАЛО ВСТАВКИ: вычитаемое с индексами и является выходной переменной                                                
									if ((IndexOf(rightrightcontent,"(") > 0) & (IndexOf(rightrightcontent,",") > 0))
									{
										int rightbracket = IndexOf(rightrightcontent,"(");
										string rightrightcontentname = rightrightcontent.substr(0, rightbracket);//Имя переменной справа от операции вычитания
										//System.Console.WriteLine($"Имя переменной справа от операции вычитания {rightrightcontentname}");
										//System.Console.ReadKey();
										//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ УМЕНЬШАЕМОГО
										string rightoperand = rightrightcontentname + "(";
										string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
										//System.Console.WriteLine($"Для операции вычитания внутри скобок вычитаемого {vnutriskobok}");
										//System.Console.ReadKey();
										vector<string> masindex = Split(vnutriskobok, ',' );
										for (string s : masindex)
										{
											//System.Console.WriteLine($"{s} - индекс вычитаемого.");//теперь нужно получить значение индексов вычитаемого
											//System.Console.ReadKey();
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов вычитаемого и определяются их значения
											{
												if (c.first == s)
												{
													rightoperand = rightoperand + vnutrperem[s] + ",";
													//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} вычитаемого.");
													//System.Console.ReadKey();
												}
											}
										}
										rightoperand = rightoperand.substr(0, rightoperand.size() - 1) + ")";
										//System.Console.WriteLine($"{leftoperand} - значение вычитаемого.");
										//System.Console.ReadKey();
										bool rightrightcontentoutp = false;
										for (auto c : outdata)// Проверяется, является ли вычитаемое выходной переменной.
										{
											if (c.first == rightoperand) { rightrightcontentoutp = true; };
										}
										string rightrightcontentvalue = " ";
										if (rightrightcontentoutp == true)
										{
											if ((IndexOf(vnutrperem[leftcontent],"{")) >= 0)
											{
												rightrightcontentvalue = outdata[rightoperand];
												if ((IndexOf(rightrightcontentvalue,"{")) >= 0)
												{
													vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightrightcontentvalue + "}";
													//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
												else
												{
													vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+rightrightcontentvalue+"\"}";
													//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
											}
										}
									}                                                
								}
								if ((IndexOf(rightcontent,"*") > 0))
								{
									int locationoperation = IndexOf(rightcontent,"*");
									string leftrightcontent = rightcontent.substr(0, locationoperation);
									//System.Console.WriteLine($"Слева от операции умножения {leftrightcontent}");
									//System.Console.ReadKey();
									string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
									//System.Console.WriteLine($"Справа от операции умножения {rightrightcontent}");
									//System.Console.ReadKey();
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,".") < 0)) //Левый сомножитель с несколькими индексами
									//if ((leftrightcontentIndexOf(,"(") > 0)) //Левый сомножитель с индексами
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции умножения
										//System.Console.WriteLine($"Имя переменной слева от операции умножения {leftrightcontentname}");
										//System.Console.ReadKey();
										string leftsomnoj = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);
										if (IndexOf(leftrightcontent,",") > 0) //Левый сомножитель с несколькими индексами
										{
											vector<string> masindex = Split(vnutriskobok, ',');
											for (string s : masindex)
											{
												//System.Console.WriteLine($"{s} - индекс левого сомножителя.");//теперь нужно получить значение индексов левого сомножителя
												//System.Console.ReadKey();
												for (auto c : vnutrperem)//ищем внутренние переменные для индексов первого сомножителя и определяются их значения
												{
													if (c.first == s)
													{
														leftsomnoj = leftsomnoj + vnutrperem[s] + ",";
														//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} левого сомножителя.");
														//System.Console.ReadKey();
													}
												}
											}
											leftsomnoj = leftsomnoj.substr(0, leftsomnoj.size() - 1) + ")";
											//System.Console.WriteLine($"{leftsomnoj} - значение левого сомножителя.");
											//System.Console.ReadKey();
											bool leftsomnojvnutp = false;
											for (auto c : vnutrperem)//определяем, является ли левый сомножитель внутренней переменной.
											{
												if (c.first == leftsomnoj)
												{
													leftsomnojvnutp = true;                                                               
												}
											}
											string leftvalue = " ";
											if (leftsomnojvnutp == true)
											{
												if (IndexOf(vnutrperem[leftsomnoj],"{") < 0)
												{
													leftvalue = "{\"op\":\"*\",\"fO\":\"" + vnutrperem[leftsomnoj]+"\",";
													//System.Console.WriteLine($"Значением 1 (не JSON )является {leftvalue}");
													//System.Console.ReadKey();
												}                                                        
												if (IndexOf(vnutrperem[leftsomnoj],"{") >= 0)
												{
													leftvalue = "{\"op\":\"*\",\"fO\":" + vnutrperem[leftsomnoj]+",";
													//System.Console.WriteLine($"Значением 2 (JSON) является {leftvalue}");
													//System.Console.ReadKey();
												}
											}
											if (leftsomnojvnutp == false)
											{
												//System.Console.WriteLine($"Левый сомножитель {leftsomnoj} не является внутренней переменной");
												//System.Console.ReadKey();
												leftvalue = "{\"op\":\"*\",\"fO\":\"" + leftsomnoj+"\",";
											}
											vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {leftvalue}");
											//System.Console.ReadKey();
										}
									}
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") < 0))
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции умножения
										//System.Console.WriteLine($"Имя переменной слева от операции умножения {leftrightcontentname}");
										//System.Console.ReadKey();                                                    
										string leftsomnoj = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);
										//System.Console.WriteLine($"Внутри скобок первого сомножителя {vnutriskobok}");
										//System.Console.ReadKey();
										bool vnutriskobokvp = false;
										for (auto c : vnutrperem)//проверяется, является ли индексом первого сомножителя внутренняя переменная
										{
											if (c.first == vnutriskobok)
											{
												vnutriskobokvp = true;
											}
										}
										if (vnutriskobokvp == true)
										{
											leftsomnoj = leftsomnoj + vnutrperem[vnutriskobok] + ")";
											for (string c : indata)
											{
												if (c == leftsomnoj)
												{
													vnutrperem[leftcontent] = "{\"op\":\"*\",\"fO\":\"" + leftsomnoj+"\",";
													//System.Console.WriteLine($"Внутренней переменной {leftcontent} присвоена первая часть значения {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
											}
										}
									}
									if ((IndexOf(rightrightcontent,"(") > 0)) //Второй сомножитель имеет хотя бы один индекс
									{
										int leftbracket = IndexOf(rightrightcontent,"(");
										string rightrightcontentname = rightrightcontent.substr(0, leftbracket);//Имя переменной справа от операции умножения
										//System.Console.WriteLine($"Имя переменной справа от операции умножения {rightrightcontentname}");
										//System.Console.ReadKey();
										string rightsomnoj = rightrightcontentname + "(";
										string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
										//System.Console.WriteLine($"D находилась в коллекции, {vnutriskobok} - индекс правого сомножителя, {vnutrperem[vnutriskobok]} - значение индекса");
										//System.Console.ReadKey();
										if (IndexOf(rightrightcontent,",") > 0) //Второй сомножитель имеет несколько индексов
										{
											vector<string> masindex = Split(vnutriskobok, ',' );
											for (string s : masindex)
											{
												//System.Console.WriteLine($"{s} - индекс правого сомножителя.");
												//System.Console.ReadKey();
												for (auto c : vnutrperem)//ищутся внутренние переменные для индексов правого сомножителя и определяются их значения
												{
													if (c.first == s)
													{
														rightsomnoj = rightsomnoj + vnutrperem[s] + ",";
														//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} правого сомножителя.");
														//System.Console.ReadKey();
													}
												}
											}
											rightsomnoj = rightsomnoj.substr(0, rightsomnoj.size() - 1) + ")";
											//System.Console.WriteLine($"{rightsomnoj} - значение правого сомножителя.");
											//System.Console.ReadKey();
										}
										if (IndexOf(rightrightcontent,",") < 0) //Второй сомножитель имеет один индекс
										{
											for (auto c : vnutrperem)//ищется внутренняя переменная для индекса правого сомножителя и определяются ее значение
											{
												if (c.first == vnutriskobok)
												{
													rightsomnoj = rightsomnoj + vnutrperem[vnutriskobok];
													//System.Console.WriteLine($"D находилась в коллекции, {vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} правого сомножителя в цикле.");
													//System.Console.ReadKey();
												}

											}
											rightsomnoj = rightsomnoj.substr(0, rightsomnoj.size()) + ")";
											//System.Console.WriteLine($"{leftcontent} находилась в коллекции, {rightsomnoj} - значение правого сомножителя.");
											//System.Console.ReadKey();
											bool outd = false;
											for (auto c : outdata)
											{
												if (c.first == rightsomnoj)
												{
													outd = true;
													rightsomnoj = outdata[rightsomnoj];
													//System.Console.WriteLine($"{rightsomnoj} - значение правого сомножителя, как выходной переменной X(j).");
													//System.Console.ReadKey();
												}
											}
										}
										bool rightsomnojvnutp = false;
										for (auto c : vnutrperem) //Проверяется, является ли второй сомножитель внутренней переменной
										{
											if (c.first == rightsomnoj)
											{
												rightsomnojvnutp = true;
											}
										}
										if (rightsomnojvnutp == true) //Второй сомножитель является внутренней переменной
										{
											if (IndexOf(vnutrperem[rightsomnoj],"{") < 0)
											{
												vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+vnutrperem[rightsomnoj]+"\"}";
												//System.Console.WriteLine($"Значением 1 (не JSON) с учетом второго сомножителя является {vnutrperem[leftcontent]}");
												//System.Console.ReadKey();
											}
											if (IndexOf(vnutrperem[rightsomnoj],"{") >= 0)
											{
												vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+vnutrperem[rightsomnoj] + "}";
												//System.Console.WriteLine($"Значением 2 (JSON) с учетом второго сомножителя является {vnutrperem[leftcontent]}");
												//System.Console.ReadKey();
											}
										}
										if (rightsomnojvnutp == false)
										{
											bool outd = false;
											for (auto c : outdata)
											{
												if (c.first == rightsomnoj)
												{
													outd = true;
													rightsomnoj = outdata[rightsomnoj];
													//System.Console.WriteLine($"{rightsomnoj} - значение правого сомножителя, как выходной переменной U(i,j).");
													//System.Console.ReadKey();
												}
											}
											if ((IndexOf(rightsomnoj,"{") == 0))
											{
												vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightsomnoj + "}";
											}
											else
											{
												vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+rightsomnoj+"\"}";
											}
											//System.Console.WriteLine($"значением внуренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
											//System.Console.ReadKey();
										}
										//System.Console.WriteLine($"D находилась в коллекции, значением внуренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
										//System.Console.ReadKey();
										//КОНЕЦ ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ВТОРОГО СОМНОЖИТЕЛЯ 
									}                                                
									string leftrightcontentvalue = " ";
									if ((IndexOf(leftrightcontent,"(") < 0)) //Левый сомножитель без индексов
									{
										bool leftrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, находится ли слева от операции умножения внутренняя переменная.
										{
											if (c.first == leftrightcontent) { leftrightcontentvp = true; };
										}
										if (leftrightcontentvp == true)
										{
											leftrightcontentvalue = vnutrperem[leftrightcontent];
											//System.Console.WriteLine($"Слева от операции умножения внутренняя переменная {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										if ((leftrightcontent.substr(0, 1) == "0") || (leftrightcontent.substr(0, 1) == "1") || (leftrightcontent.substr(0, 1) == "2") || (leftrightcontent.substr(0, 1) == "3") || (leftrightcontent.substr(0, 1) == "4") || (leftrightcontent.substr(0, 1) == "5") || (leftrightcontent.substr(0, 1) == "6") ||
										   (leftrightcontent.substr(0, 1) == "7") || (leftrightcontent.substr(0, 1) == "8") || (leftrightcontent.substr(0, 1) == "9"))
										{
											//System.Console.WriteLine($"Значение первого сомножителя является числом {leftrightcontent}");
											//System.Console.ReadKey();
											leftrightcontentvalue = leftrightcontent;
										}
										bool leftrightcontentinp = false;
										for (string c : indata)// Проверяется, находится ли слева от операции умножения входная переменная.
										{
											if (c == leftrightcontent) { leftrightcontentinp = true; };
										}
										if (leftrightcontentinp == true)
										{
											leftrightcontentvalue = leftrightcontent;
											//System.Console.WriteLine($"Слева от операции умножения входная переменная {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();                                                                                                               
										}
									}
									if ((IndexOf(rightrightcontent,"(") < 0)) //Правый сомножитель без индексов
									{
										//System.Console.WriteLine($"Имя переменной справа от операции умножения {rightrightcontent}");
										//System.Console.ReadKey();
										if ((rightrightcontent.substr(0, 1) == "0") || (rightrightcontent.substr(0, 1) == "1") || (rightrightcontent.substr(0, 1) == "2") || (rightrightcontent.substr(0, 1) == "3") || (rightrightcontent.substr(0, 1) == "4") || (rightrightcontent.substr(0, 1) == "5") || (rightrightcontent.substr(0, 1) == "6") ||
										   (rightrightcontent.substr(0, 1) == "7") || (rightrightcontent.substr(0, 1) == "8") || (rightrightcontent.substr(0, 1) == "9"))
										{
											//System.Console.WriteLine($"Значение второго сомножителя является числом {rightrightcontent}");
											//System.Console.ReadKey();
											string rightvalue = to_string((stoi(leftrightcontentvalue)) * (stoi(rightrightcontent)));
											vnutrperem[leftcontent] = rightvalue;
											//System.Console.WriteLine($"Внутренняя переменная {leftcontent} изменена в коллекции, ее значением является {rightvalue}");
											//System.Console.ReadKey();
										}
										bool rightrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, находится ли справа от операции умножения внутренняя переменная.
										{
											if (c.first == rightrightcontent)
											{ rightrightcontentvp = true; };
										}
										if (rightrightcontentvp == true)
										{
											string rightvalue = "{\"op\":\"*\",\"fO\":\"" + leftrightcontentvalue + "\",\"sO\":" + vnutrperem[rightrightcontent] + "}";
											//System.Console.WriteLine($"Значение левого сомножителя {leftrightcontentvalue}");
											//System.Console.WriteLine($"Значение внутренней переменной {leftcontent}");
											vnutrperem[leftcontent] = rightvalue;
											//System.Console.WriteLine($"стало значением {rightvalue}");
											//System.Console.ReadKey();
										}
										bool rightrightcontentinp = false;
										for (string c : indata)// Проверяется, находится ли справа от операции умножения входная переменная.
										{
											if (c == rightrightcontent) { rightrightcontentinp = true; };
										}
										if (rightrightcontentinp == true)
										{
											string rightvalue = "{\"op\":\"*\",\"fO\":\"" + leftrightcontentvalue + "\",\"sO\":\"" + rightrightcontent+"\"}";
											//System.Console.WriteLine($"Значение левого сомножителя {leftrightcontentvalue}");
											//System.Console.WriteLine($"Значение правого сомножителя {rightrightcontent}");
											vnutrperem[leftcontent] = rightvalue;
											//System.Console.WriteLine($"Во внутреннюю переменную {leftcontent} записано значение {rightvalue}");
											//System.Console.ReadKey();                                                                                                               
										}
									}
								}
								if ((IndexOf(rightcontent,"/") > 0))
								{
									int locationoperation = IndexOf(rightcontent,"/");
									string leftrightcontent = rightcontent.substr(0, locationoperation);
									//System.Console.WriteLine($"Слева от операции деления {leftrightcontent}");
									//System.Console.ReadKey();
									string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
									//System.Console.WriteLine($"Справа от операции деления {rightrightcontent}");
									//System.Console.ReadKey();
									string leftrightcontentvalue = "";
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") < 0))//делимое с одним индексом
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции деления
										//System.Console.WriteLine($"Имя переменной слева от операции деления {leftrightcontentname}");
										//System.Console.ReadKey();
										string leftsomnoj = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);

										leftsomnoj = leftsomnoj + vnutrperem[vnutriskobok] + ")";
										//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} делимого.");
										//System.Console.ReadKey();

										bool leftrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли делимое внутренней переменной.
										{
											if (c.first == leftsomnoj) { leftrightcontentvp = true; };
										}
										if (leftrightcontentvp == true)
										{
											leftrightcontentvalue = vnutrperem[leftsomnoj];
											//System.Console.WriteLine($"Слева от операции деления {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										string leftvalue = "{\"op\":\"/\",\"fO\":" + leftrightcontentvalue+",";
										vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
										//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {leftvalue}");
										//System.Console.ReadKey();                                                    
									}
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") > 0))//делимое с несколькими индексами
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции деления
										//System.Console.WriteLine($"Имя переменной слева от операции деления {leftrightcontentname}");
										//System.Console.ReadKey();
										string leftdel = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);
										vector<string> masindex = Split(vnutriskobok, ',');
										for (string s : masindex)
										{
											//System.Console.WriteLine($"{s} - индекс делимого.");
											//System.Console.ReadKey();
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов делимого и определяются их значения
											{
												if (c.first == s)
												{
													leftdel = leftdel + vnutrperem[s] + ",";
													//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} делимого.");
													//System.Console.ReadKey();
												}
											}
										}
										leftdel = leftdel.substr(0, leftdel.size() - 1) + ")";
										//System.Console.WriteLine($"{leftdel} - значение делимого.");
										//System.Console.ReadKey();
										bool leftrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли делимое внутренней переменной.
										{
											if (c.first == leftdel) { leftrightcontentvp = true; };
										}
										if (leftrightcontentvp == true)
										{
											leftrightcontentvalue = vnutrperem[leftdel];
											//System.Console.WriteLine($"Слева от операции деления внутрення переменная {leftdel}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										else
										{
											leftrightcontentvalue = leftdel;
											//System.Console.WriteLine($"Слева от операции деления {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										bool lrcvinp = false;
										for (string c : indata)//проверяем: делимое является входной переменной?
										{                                                                                          
											if (c == leftrightcontentvalue)
											{
												lrcvinp = true;
												//System.Console.WriteLine($"Делимое {leftrightcontentvalue} является входной переменной?");
												//System.Console.ReadKey();
											}
										}
										if ((lrcvinp == false) & (IndexOf(leftrightcontentvalue,"{") < 0))
										{
											vnutrperem[leftcontent] = leftrightcontentvalue;
										}
										string leftvalue = " ";
										if (lrcvinp == true)
										{
											leftvalue = "{\"op\":\"/\",\"fO\":\"" + leftrightcontentvalue+"\",";
											vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {leftvalue}");
											//System.Console.ReadKey();
										}
										if (IndexOf(leftrightcontentvalue,"{") >= 0)
										{
											leftvalue = "{\"op\":\"/\",\"fO\":" + leftrightcontentvalue+",";
											vnutrperem[leftcontent] = leftvalue;//изменено значение внутренней переменной
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} изменена, ее значением является {leftvalue}");
											//System.Console.ReadKey();
										}                                                    
									}                                              
									if ((IndexOf(leftrightcontent,"(") < 0))//делимое
									{
										bool leftrightcontentinp = false;
										for (string c : indata)// Проверяется, находится ли слева от операции деления входная переменная.
										{
											if (c == leftrightcontent) { leftrightcontentinp = true; };
										}
										if (leftrightcontentinp == true)
										{
											vnutrperem[leftcontent] = "{\"op\":\"/\",\"fO\":\"" + leftrightcontentvalue+"\",";
											//System.Console.WriteLine($"Слева от операции деления {leftrightcontent}, во внутреннюю переменную {leftcontent} записано значение {vnutrperem[leftcontent]}");
											//System.Console.ReadKey();
										}
									}                                                
									if ((IndexOf(rightrightcontent,"(") < 0))//делитель без индексов
									{
										//System.Console.WriteLine($"Имя переменной справа от операции деления {rightrightcontent}");
										//System.Console.ReadKey();
										if ((rightrightcontent.substr(0, 1) == "0") || (rightrightcontent.substr(0, 1) == "1") || (rightrightcontent.substr(0, 1) == "2") || (rightrightcontent.substr(0, 1) == "3") || (rightrightcontent.substr(0, 1) == "4") || (rightrightcontent.substr(0, 1) == "5") || (rightrightcontent.substr(0, 1) == "6") ||
										   (rightrightcontent.substr(0, 1) == "7") || (rightrightcontent.substr(0, 1) == "8") || (rightrightcontent.substr(0, 1) == "9"))
										{
											//System.Console.WriteLine($"Значение делителя является числом {rightrightcontent}");
											//System.Console.ReadKey();
											string rightvalue = to_string((stoi(leftrightcontentvalue)) + (stoi(rightrightcontent)));
											vnutrperem[leftcontent] = rightvalue;
											//System.Console.WriteLine($"Значение внутренней переменной {leftcontent} обновлено и составляет {rightvalue}");
											//System.Console.ReadKey();
										}
										bool rightrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли делитель внутренней переменной.
										{
											if (c.first == rightrightcontent) { rightrightcontentvp = true; };
										}
										string rightrightcontentvalue = " ";
										if (rightrightcontentvp == true)
										{
											if ((IndexOf(vnutrperem[leftcontent],"{")) >= 0)
											{
												rightrightcontentvalue = vnutrperem[rightrightcontent];
												if ((IndexOf(rightrightcontentvalue,"{")) >= 0)
												{
													vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightrightcontentvalue + "}";
													//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
												else
												{
													vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+rightrightcontentvalue+"\"}";
													//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
											}                                                
										 
										}
									}
									if ((IndexOf(rightrightcontent,"(") > 0))//делитель с индексами
									{
										int leftbracket = IndexOf(rightrightcontent,"(");
										string rightrightcontentname = rightrightcontent.substr(0, leftbracket);//Имя переменной справа от операции деления
										//System.Console.WriteLine($"Имя переменной справа от операции деления {rightrightcontentname}");
										//System.Console.ReadKey();
										//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ДЕЛИТЕЛЯ
										string rightsomnoj = rightrightcontentname + "(";
										string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
										if (IndexOf(rightrightcontent,",") > 0)//делитель с несколькими индексами
										{
											vector<string> masindex = Split(vnutriskobok, ',' );
											for (string s : masindex)
											{
												//System.Console.WriteLine($"{s} - индекс делителя.");
												//System.Console.ReadKey();
												for (auto c : vnutrperem)//ищутся внутренние переменные для индексов делителя и определяются их значения
												{
													if (c.first == s)
													{
														rightsomnoj = rightsomnoj + vnutrperem[s] + ",";
														//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} делителя.");
														//System.Console.ReadKey();
													}
												}
											}
											rightsomnoj = rightsomnoj.substr(0, rightsomnoj.size() - 1) + ")";
											//System.Console.WriteLine($"{rightsomnoj} - значение делителя.");
											//System.Console.ReadKey();
										}
										if (IndexOf(rightrightcontent,",") < 0)//делитель с одним индексом
										{
											for (auto c : vnutrperem)//ищется индекс делителя и определяется его значение
											{
												if (c.first == vnutriskobok)
												{
													rightsomnoj = rightsomnoj + vnutrperem[vnutriskobok];
													//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} делителя.");
													//System.Console.ReadKey();
												}
											}
											rightsomnoj = rightsomnoj.substr(0, rightsomnoj.size()) + ")";
											//System.Console.WriteLine($"{rightsomnoj} - значение делителя.");
											//System.Console.ReadKey();
										}
										//vnutrperem[leftcontent] = vnutrperem[leftcontent] + $"{rightsomnoj})";//изменение значения внутренней переменной
										vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+rightsomnoj+"\"}";
										//System.Console.WriteLine($"Значением внуренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
										//System.Console.ReadKey();
										//КОНЕЦ ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ДЕЛИТЕЛЯ 
									}
								}
								if ((IndexOf(rightcontent,"abs(") == 0))
								{
									int locationoperation = IndexOf(rightcontent,"abc");
									string rightrightcontent = rightcontent.substr(locationoperation + 4, rightcontent.size() - locationoperation - 4);
									//System.Console.WriteLine($"Справа от операции abs {rightrightcontent}");
									//System.Console.ReadKey();
									string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
									//System.Console.WriteLine($"Внутри скобок операции abs {vnutriskobok}");
									//System.Console.ReadKey();
									bool vnutriskobokvp = false;
									for (auto c : vnutrperem)//определяется значение внутри скобок операции abs
									{
										if (c.first == vnutriskobok)
										{
											vnutriskobokvp = true;
										}
									}
									if ((vnutriskobokvp == true))
									{
										string rightvalue = "{\"op\":\"abs\",\"od\":" + vnutrperem[vnutriskobok] + "}";
										vnutrperem[leftcontent] = rightvalue;//добавление значения внутренней переменной
										//System.Console.WriteLine($"Внуренняя переменная {leftcontent} добавлена в коллекцию, ее значением является {rightvalue}");
										//System.Console.ReadKey();
									}
								}
								if ((IndexOf(rightcontent,"sqrt(") == 0))
								{
									int locationoperation = IndexOf(rightcontent,"sqrt");
									string rightrightcontent = rightcontent.substr(locationoperation + 4, rightcontent.size() - locationoperation - 4);
									//System.Console.WriteLine($"Справа от операции sqrt {rightrightcontent}");
									//System.Console.ReadKey();                                                                                              

									string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
									//System.Console.WriteLine($"Внутри скобок операции sqrt {vnutriskobok}");
									//System.Console.ReadKey();
									bool vnutriskobokvp = false;
									for (auto c : vnutrperem)//определяется значение внутри скобок операции abs
									{
										if (c.first == vnutriskobok)
										{
											vnutriskobokvp = true;
										}
									}
									if ((vnutriskobokvp == true))
									{
										string rightvalue = "{\"op\":\"sqrt\",\"od\":" + vnutrperem[vnutriskobok] + "}";
										vnutrperem[leftcontent] = rightvalue;//Запись значения внутренней переменной
										//System.Console.WriteLine($"Значение внуренней переменной {leftcontent} записано в коллекцию, оно равно {rightvalue}");
										//System.Console.ReadKey();
									}
								}
							}
							if (vnutp == false)
								{
								string rightcontent = content.substr(assignmentcharacter + 1, content.size() - assignmentcharacter - 1);
								//System.Console.WriteLine($"Справа от присваивания {rightcontent}");
								//System.Console.ReadKey();
								if ((IndexOf(rightcontent,"+") < 0) & (IndexOf(rightcontent,"-") < 0) & (IndexOf(rightcontent,"*") < 0) & (IndexOf(rightcontent,"/") < 0) & (IndexOf(rightcontent,"abs") < 0) & (IndexOf(rightcontent,"sqrt") < 0))
								{
									//System.Console.WriteLine($"{rightcontent} не содержит операцию");
									//System.Console.ReadKey();
									if ((rightcontent.substr(0, 1) == "0") || (rightcontent.substr(0, 1) == "1") || (rightcontent.substr(0, 1) == "2") || (rightcontent.substr(0, 1) == "3") || (rightcontent.substr(0, 1) == "4") || (rightcontent.substr(0, 1) == "5") || (rightcontent.substr(0, 1) == "6") ||
									   (rightcontent.substr(0, 1) == "7") || (rightcontent.substr(0, 1) == "8") || (rightcontent.substr(0, 1) == "9"))
									{
										vnutrperem[leftcontent]=rightcontent;
										//System.Console.WriteLine($"Внуренняя переменная {leftcontent} добавлена в коллекцию, ее значением является число.");
										//System.Console.ReadKey();
										//for (KeyValuePair<string, string> keyValue : vnutrperem)
										//{
										//    Console.WriteLine(keyValue.Key + " - " + keyValue.Value);
										//}
										//System.Console.ReadKey();
									}
									bool vnutrpr = false; //справа внутренняя переменная без индекса?
									for (auto c : vnutrperem)
									{
										if (c.first == rightcontent)
										{
											vnutrpr = true;                                                        
										}
									}
									if (vnutrpr == true)
									{
										vnutrperem[leftcontent]=vnutrperem[rightcontent];
										//System.Console.WriteLine($"Внутренняя переменная {leftcontent} добавлена в коллекцию, ее значением является {vnutrperem[rightcontent]}.");
										//System.Console.ReadKey();
									}
									if ((IndexOf(rightcontent,"(") > 0) & (IndexOf(rightcontent,",") < 0))//один индекс
									{
										int rightbracket = IndexOf(rightcontent,"(");
										string rightcontentname = rightcontent.substr(0, rightbracket);//имя переменной слева от скобки (
										string rightotprisv = rightcontentname + "(";
										string vnutriskobok = rightcontent.substr(IndexOf(rightcontent,"(") + 1, IndexOf(rightcontent,")") - IndexOf(rightcontent,"(") - 1);
										for (auto c : vnutrperem)//ищется внутренняя переменная для индекса переменной слева от знака присваивания и определяется ее значение
										{
											if (c.first == vnutriskobok)
											{
												rightotprisv = rightotprisv + vnutrperem[vnutriskobok];
												//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} переменной справа от знака присваивания.");
												//System.Console.ReadKey();
											}
										}
										rightotprisv = rightotprisv.substr(0, rightotprisv.size()) + ")";
										bool vnpr = false;
										for (auto c : vnutrperem)//определяется, является ли справа от знака присваивания переменная с одним индексом внутренней переменной
										{
											if (c.first == rightotprisv)
											{
												vnpr = true;
												//System.Console.WriteLine($"Справа от присваивания внутренняя переменная {rightotprisv}.");
												//System.Console.ReadKey();
											}
										}
										bool outpr = false;
										for (auto c : outdata)//определяется, является ли справа от знака присваивания переменная с одним индексом выходной переменной
										{
											if (c.first == rightotprisv)
											{
												outpr = true;
												//System.Console.WriteLine($"Справа от присваивания выходная переменная {rightotprisv}.");
												//System.Console.ReadKey();
											}
										}
										string rightotprisvv = " ";
										if (vnpr == true)
										{
											rightotprisvv = vnutrperem[rightotprisv];
										}
										if (outpr == true)
										{
											rightotprisvv = outdata[rightotprisv];
										}
										if ((rightotprisvv.substr(0, 1) == "0") || (rightotprisvv.substr(0, 1) == "1") || (rightotprisvv.substr(0, 1) == "2") || (rightotprisvv.substr(0, 1) == "3") || (rightotprisvv.substr(0, 1) == "4") || (rightotprisvv.substr(0, 1) == "5") || (rightotprisvv.substr(0, 1) == "6") ||
										   (rightotprisvv.substr(0, 1) == "7") || (rightotprisvv.substr(0, 1) == "8") || (rightotprisvv.substr(0, 1) == "9"))
										{
											rightcontent = rightotprisvv;
										}
										else
										{
											rightcontent = rightotprisv;
										}
										vnutrperem[leftcontent]=rightcontent;
										//System.Console.WriteLine($"Добавлена внутренняя переменная {leftcontent}, в нее записано значение {rightcontent}.");
										//System.Console.ReadKey();
										//for (KeyValuePair<string, string> keyValue : vnutrperem)
										//{
										//    Console.WriteLine(keyValue.Key + " - " + keyValue.Value);
										//}
										//System.Console.ReadKey();
									}
									if ((IndexOf(rightcontent,"(") > 0) & (IndexOf(rightcontent,",") > 0))//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРЕМЕННОЙ СПРАВА ОТ ЗНАКА ПРИСВАИВАНИЯ
									{
										int leftbracket = IndexOf(rightcontent,"(");
										string rightcontentname = rightcontent.substr(0, leftbracket);//имя переменной слева от скобки (
										//System.Console.WriteLine($"Имя переменной справа от знака присваивания {rightcontentname}");
										//System.Console.ReadKey();
										string rightotprisv = rightcontentname + "(";
										string vnutriskobok = rightcontent.substr(IndexOf(rightcontent,"(") + 1, IndexOf(rightcontent,")") - IndexOf(rightcontent,"(") - 1);
											vector<string> masindex = Split(vnutriskobok, ',' );
											for (string s : masindex)
											{
												//System.Console.WriteLine($"{s} - индекс переменной слева от знака присваивания.");//теперь нужно получить значение индексов переменной слева от знака присваивания
												//System.Console.ReadKey();
												for (auto c : vnutrperem)//ищутся внутренние переменные для индексов переменной слева от знака присваивания и определяются их значения
												{
													if (c.first == s)
													{
													rightotprisv = rightotprisv + vnutrperem[s] + ",";
														//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} переменной слева от знака присваивания.");
														//System.Console.ReadKey();
													}
												}
											}
											rightotprisv = rightotprisv.substr(0, rightotprisv.size() - 1) + ")";
											rightcontent = rightotprisv;
											bool rightcontentvnp = false;
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов переменной слева от знака присваивания и определяются их значения
											{
												if (c.first == rightcontent)
												{
													rightcontentvnp = true;
													//System.Console.WriteLine($"Справа от знака присваивания внутренняя переменная {rightcontent}.");
													//System.Console.ReadKey();
												}
											}
										if (rightcontentvnp == true)
										{   vnutrperem[leftcontent]=vnutrperem[rightcontent];
											//System.Console.WriteLine($"Внутренняя переменная {leftcontent} добавлена в коллекцию. Ее значением является {vnutrperem[rightcontent]}.");
											//System.Console.ReadKey();
										}
										else
										{
											vnutrperem[leftcontent]=rightcontent;
											//System.Console.WriteLine($"Внутренняя переменная {leftcontent} добавлена в коллекцию. Ее значением является {rightcontent}.");
											//System.Console.ReadKey();
										}
									}                                                
								}
								if ((IndexOf(rightcontent,"+") > 0))
								{
									int locationoperation = IndexOf(rightcontent,"+");
									string leftrightcontent = rightcontent.substr(0, locationoperation);
									//System.Console.WriteLine($"Слева от операции сложения {leftrightcontent}");
									//System.Console.ReadKey();
									string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
									//System.Console.WriteLine($"Справа от операции сложения {rightrightcontent}");
									//System.Console.ReadKey();
									bool leftvnutrp = false;
									bool rightvnutrp = false;
									if ((IndexOf(leftrightcontent,"(") < 0) & (IndexOf(rightrightcontent,"(") < 0))
									{                                         
										for (auto c : vnutrperem)
										{
											if (c.first == leftrightcontent)
											{
												leftvnutrp = true;
												//System.Console.WriteLine($"Внутренняя переменная {leftrightcontent} есть в коллекции.");
												//System.Console.ReadKey();
											}
										}                                                    
										for (auto c : vnutrperem)
										{
											if (c.first == rightrightcontent)
											{
												rightvnutrp = true;
												//System.Console.WriteLine($"Внутренняя переменная {rightrightcontent} есть в коллекции.");
												//System.Console.ReadKey();
											}
										}
										if ((leftvnutrp == true) & (rightvnutrp == true))
										{
											if ((vnutrperem[leftrightcontent].substr(0, 1) == "0") || (vnutrperem[leftrightcontent].substr(0, 1) == "1") || (vnutrperem[leftrightcontent].substr(0, 1) == "2") || (vnutrperem[leftrightcontent].substr(0, 1) == "3") || (vnutrperem[leftrightcontent].substr(0, 1) == "4") || (vnutrperem[leftrightcontent].substr(0, 1) == "5") || (vnutrperem[leftrightcontent].substr(0, 1) == "6") ||
											(vnutrperem[leftrightcontent].substr(0, 1) == "7") || (vnutrperem[leftrightcontent].substr(0, 1) == "8") || (vnutrperem[leftrightcontent].substr(0, 1) == "9"))
											{                                                            
												if ((vnutrperem[rightrightcontent].substr(0, 1) == "0") || (vnutrperem[rightrightcontent].substr(0, 1) == "1") || (vnutrperem[rightrightcontent].substr(0, 1) == "2") || (vnutrperem[rightrightcontent].substr(0, 1) == "3") || (vnutrperem[rightrightcontent].substr(0, 1) == "4") || (vnutrperem[rightrightcontent].substr(0, 1) == "5") || (vnutrperem[rightrightcontent].substr(0, 1) == "6") ||
												(vnutrperem[rightrightcontent].substr(0, 1) == "7") || (vnutrperem[rightrightcontent].substr(0, 1) == "8") || (vnutrperem[rightrightcontent].substr(0, 1) == "9"))
												{
													string rightvalue = to_string((stoi(vnutrperem[leftrightcontent])) + (stoi(vnutrperem[rightrightcontent])));
													vnutrperem[leftcontent]=rightvalue;
													//System.Console.WriteLine($"Добавлена внутренняя переменная p, ее значение {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
											}
											if (IndexOf(vnutrperem[leftrightcontent],"{") >= 0)
											{
												if (IndexOf(vnutrperem[rightrightcontent],"{") >= 0)
												{
													string rightvalue = "{\"op\":\"+\",\"fO\":" + vnutrperem[leftrightcontent] + ",\"sO\":" + vnutrperem[rightrightcontent] + "}";
													vnutrperem[leftcontent]=rightvalue;
													//System.Console.WriteLine($"Добавлена внутренняя переменная {leftcontent}, ее значение {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
											}
										}
									}
								}
								if ((IndexOf(rightcontent,"-") > 0))
								{
									int locationoperation = IndexOf(rightcontent,"-");
									string leftrightcontent = rightcontent.substr(0, locationoperation);
									//System.Console.WriteLine($"Слева от операции вычитания {leftrightcontent}");
									//System.Console.ReadKey();
									string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
									//System.Console.WriteLine($"Справа от операции вычитания {rightrightcontent}");
									//System.Console.ReadKey();
									if ((IndexOf(leftrightcontent,"(") > 0))
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции вычитания
										//System.Console.WriteLine($"Имя переменной слева от операции вычитания {leftrightcontentname}");
										//System.Console.ReadKey();
										//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ УМЕНЬШАЕМОГО
										string leftoperand = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);
										//System.Console.WriteLine($"Для операции вычитания внутри скобок уменьшаемого {vnutriskobok}");
										//System.Console.ReadKey();
										vector<string> masindex = Split(vnutriskobok, ',');
										for (string s : masindex)
										{
											//System.Console.WriteLine($"{s} - индекс уменьшаемого.");//теперь нужно получить значение индексов уменьшаемого
											//System.Console.ReadKey();
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов уменьшаемого и определяются их значения
											{
												if (c.first == s)
												{
													leftoperand = leftoperand + vnutrperem[s] + ",";
													//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} уменьшаемого.");
													//System.Console.ReadKey();
												}
											}
										}
										leftoperand = leftoperand.substr(0, leftoperand.size() - 1) + ")";
										//System.Console.WriteLine($"{leftoperand} - значение уменьшаемого.");
										//System.Console.ReadKey();     
										string leftrightcontentvalue = " ";
										bool leftrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли уменьшаемое внутренней переменной.
										{
											if (c.first == leftoperand) { leftrightcontentvp = true; };
										}
										if (leftrightcontentvp == true)
										{
											leftrightcontentvalue = vnutrperem[leftoperand];
											//System.Console.WriteLine($"Слева от операции вычитания {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										else
										{
											leftrightcontentvalue = leftoperand;
											//System.Console.WriteLine($"Слева от операции деления {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										bool lrcvinp = false;
										for (string c : indata)//проверяем: уменьшаемое является входной переменной?
										{
											if (c == leftrightcontentvalue)
											{
												lrcvinp = true;
												//System.Console.WriteLine($"Делимое {leftrightcontentvalue} является входной переменной?");
												//System.Console.ReadKey();
											}
										}
										if ((lrcvinp == false) & (IndexOf(leftrightcontentvalue,"{") < 0))
										{
											vnutrperem[leftcontent]=leftrightcontentvalue;
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} добавлена, ее значением является {leftrightcontentvalue} ");
											//System.Console.ReadKey();
										}
										string leftvalue = " ";
										if (lrcvinp == true)
										{
											leftvalue = "{\"op\":\"-\",\"fO\":\"" + leftrightcontentvalue + "\",";
											vnutrperem[leftcontent]=leftvalue;//Добавлено значение внутренней переменной
											//System.Console.WriteLine($"Внутренняя переменная {leftcontent} добавлена, ее значением является {leftvalue}");
											//System.Console.ReadKey();
										}
										if (IndexOf(leftrightcontentvalue,"{") >= 0)
										{
											leftvalue = "{\"op\":\"-\",\"fO\":" + leftrightcontentvalue+",";
											vnutrperem[leftcontent]=leftvalue;//Добавлено значение внутренней переменной
											//System.Console.WriteLine($"Внутренняя переменная {leftcontent} добавлена, ее значением является {leftvalue}");
											//System.Console.ReadKey();
										}                                                    
									}
									if ((IndexOf(leftrightcontent,"(") < 0))//уменьшаемое без индексов
									{
										string leftrightcontentvalue = " ";
										if ((leftrightcontent.substr(0, 1) == "0") || (leftrightcontent.substr(0, 1) == "1") || (leftrightcontent.substr(0, 1) == "2") || (leftrightcontent.substr(0, 1) == "3") || (leftrightcontent.substr(0, 1) == "4") || (leftrightcontent.substr(0, 1) == "5") || (leftrightcontent.substr(0, 1) == "6") ||
										   (leftrightcontent.substr(0, 1) == "7") || (leftrightcontent.substr(0, 1) == "8") || (leftrightcontent.substr(0, 1) == "9"))
										{
											//System.Console.WriteLine($"Уменьшаемое является числом {leftrightcontent}");
											//System.Console.ReadKey();
											leftrightcontentvalue = leftrightcontent;
										}
										bool leftrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли уменьшаемое внутренней переменной.
										{
											if (c.first == leftrightcontent) { leftrightcontentvp = true; };
										}
										if (leftrightcontentvp == true)//Уменьшаемое является внутренней переменной
										{
											leftrightcontentvalue = vnutrperem[leftrightcontent];
											//System.Console.WriteLine($"Слева от операции вычитания {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										string leftvalue = " ";
										if (IndexOf(leftrightcontentvalue,"{") >= 0)
										{
											leftvalue = "{\"op\":\"-\",\"fO\":" + leftrightcontentvalue+",";
											vnutrperem[leftcontent]=leftvalue;//Добавлено значение внутренней переменной
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} добавлена, ее значением является {leftvalue}");
											//System.Console.ReadKey();
										}
										else
										{
											leftvalue = "{\"op\":\"-\",\"fO\":\"" + leftrightcontentvalue+"\",";
											vnutrperem[leftcontent]=leftvalue;//Добавлено значение внутренней переменной
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} добавлена, ее значением является {leftvalue}");
											//System.Console.ReadKey();
										}
									}
									if ((IndexOf(rightrightcontent,"(") > 0))
									{
										int rightbracket = IndexOf(rightrightcontent,"(");
										string rightrightcontentname = rightrightcontent.substr(0, rightbracket);//Имя переменной справа от операции вычитания
										//System.Console.WriteLine($"Имя переменной справа от операции вычитания {rightrightcontentname}");
										//System.Console.ReadKey();
										//НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ УМЕНЬШАЕМОГО
										string rightoperand = rightrightcontentname + "(";
										string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
										//System.Console.WriteLine($"Для операции вычитания внутри скобок вычитаемого {vnutriskobok}");
										//System.Console.ReadKey();
										vector<string> masindex = Split(vnutriskobok, ',' );
										for (string s : masindex)
										{
											//System.Console.WriteLine($"{s} - индекс вычитаемого.");//теперь нужно получить значение индексов вычитаемого
											//System.Console.ReadKey();
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов вычитаемого и определяются их значения
											{
												if (c.first == s)
												{
													rightoperand = rightoperand + vnutrperem[s] + ",";
													//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} вычитаемого.");
													//System.Console.ReadKey();
												}
											}
										}
										rightoperand = rightoperand.substr(0, rightoperand.size() - 1) + ")";
										//System.Console.WriteLine($"{leftoperand} - значение вычитаемого.");
										//System.Console.ReadKey();
										bool rightrightcontentoutp = false;
										for (auto c : outdata)// Проверяется, является ли вычитаемое выходной переменной.
										{
											if (c.first == rightoperand) { rightrightcontentoutp = true; };
										}
										string rightrightcontentvalue = " ";
										if (rightrightcontentoutp == true)
										{
											if ((IndexOf(vnutrperem[leftcontent],"{")) >= 0)
											{
												rightrightcontentvalue = outdata[rightoperand];
												if ((IndexOf(rightrightcontentvalue,"{")) >= 0)
												{
													vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightrightcontentvalue + "}";
													//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
												else
												{
													vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+rightrightcontentvalue+"\"}";
													//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();
												}
											}
										}
									}
										if ((IndexOf(rightrightcontent,"(") < 0))//вычитаемое без индексов
									{
										bool rightrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, является ли вычитаемое внутренней переменной.
										{
											if (c.first == rightrightcontent) { rightrightcontentvp = true; };
										}                                                        
											string rightrightcontentvalue = " ";
											if (rightrightcontentvp == true)
											{
												if ((IndexOf(vnutrperem[leftcontent],"{")) >= 0)
												{
													rightrightcontentvalue = vnutrperem[rightrightcontent];
													if ((IndexOf(rightrightcontentvalue,"{")) >= 0)
													{
														vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightrightcontentvalue + "}";
														//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
														//System.Console.ReadKey();
													}
													else
													{
														vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+rightrightcontentvalue+"\"}";
														//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
														//System.Console.ReadKey();
													}
												}
											}
										bool rightrightcontentinp = false;
										for (string c : indata)// Проверяется, является ли вычитаемое входной переменной.
										{
											if (c == rightrightcontent) { rightrightcontentinp = true; };
										}                                                    
										if (rightrightcontentinp == true)
										{                                                        
													vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+rightrightcontent+"\"}";
													//System.Console.WriteLine($"Значением внутренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
													//System.Console.ReadKey();                                                            
										}
									}
								}
								if ((IndexOf(rightcontent,"-") == 0))//операция унарный минус
								{
									int locationoperation = IndexOf(rightcontent,"-");
									string leftrightcontent = rightcontent.substr(0, locationoperation);
									//System.Console.WriteLine($"Слева от операции унарный минус {leftrightcontent}");
									//System.Console.ReadKey();
									string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
									//System.Console.WriteLine($"Справа от операции унарный минус {rightrightcontent}");
									//System.Console.ReadKey(); 
									if ((IndexOf(rightrightcontent,"(") < 0))//вычитаемое не содержит индексов
									{
										bool rightrightcontentinp = false;
										for (string c : indata)// Проверяется, находится ли справа от операции унарного минуса входная переменная.
										{
											if (c == rightrightcontent)
											{ rightrightcontentinp = true; };
										}
										if (rightrightcontentinp == true)
										{                                                       
											string value = "{\"op\":\"-\",\"od\":\"" + rightrightcontent+"\"}";
											vnutrperem[leftcontent]=value;
											//System.Console.WriteLine($"Добавлена входная переменная {leftcontent}, в нее зпаписано значение {value}");
											//System.Console.ReadKey();
										}
									}
								}
								if ((IndexOf(rightcontent,"*") > 0))
								{
									int locationoperation = IndexOf(rightcontent,"*");
									string leftrightcontent = rightcontent.substr(0, locationoperation);
									//System.Console.WriteLine($"Слева от операции умножения {leftrightcontent}");
									//System.Console.ReadKey();
									string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
									//System.Console.WriteLine($"Справа от операции умножения {rightrightcontent}");
									//System.Console.ReadKey();
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") > 0))
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции умножения
										//System.Console.WriteLine($"Имя переменной слева от операции умножения {leftrightcontentname}");
										//System.Console.ReadKey();
										string leftsomnoj = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);
										//System.Console.WriteLine($"Для операции умножения внутри скобок первого сомножителя {vnutriskobok}");
										//System.Console.ReadKey();
										vector<string> masindex = Split(vnutriskobok, ',' );
										for (string s : masindex)
										{
											//System.Console.WriteLine($"{s} - индекс левого сомножителя.");//теперь нужно получить значение индексов левого сомножителя
											//System.Console.ReadKey();
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов первого сомножителя и определяются их значения
											{
												if (c.first == s)
												{
													leftsomnoj = leftsomnoj + vnutrperem[s] + ",";
													//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} левого сомножителя.");
													//System.Console.ReadKey();
												}
											}
										}
										leftsomnoj = leftsomnoj.substr(0, leftsomnoj.size() - 1) + ")";
										//System.Console.WriteLine($"{leftsomnoj} - значение левого сомножителя.");
										//System.Console.ReadKey();
										//string leftvalue = $"*({leftsomnoj},";
										string leftvalue = "{\"op\":\"*\",\"fO\":\"" + leftsomnoj+"\",";
										vnutrperem[leftcontent]=leftvalue;//добавление значения внутренней переменной
										//System.Console.WriteLine($"Внуренняя переменная {leftcontent} добавлена в коллекцию, ее значением является {leftvalue}");
										//System.Console.ReadKey();                                                    
									}
									if ((IndexOf(leftrightcontent,"(") > 0) & (IndexOf(leftrightcontent,",") < 0))
									{
										int leftbracket = IndexOf(leftrightcontent,"(");
										string leftrightcontentname = leftrightcontent.substr(0, leftbracket);//Имя переменной слева от операции умножения
										//System.Console.WriteLine($"Имя переменной слева от операции умножения {leftrightcontentname}");
										//System.Console.ReadKey();                                                    
										string leftsomnoj = leftrightcontentname + "(";
										string vnutriskobok = leftrightcontent.substr(IndexOf(leftrightcontent,"(") + 1, IndexOf(leftrightcontent,")") - IndexOf(leftrightcontent,"(") - 1);
										//System.Console.WriteLine($"Внутри скобок первого сомножителя {vnutriskobok}");
										//System.Console.ReadKey();
										bool vnutriskobokvp = false;
										for (auto c : vnutrperem)//проверяется, является ли индексом первого сомножителя внутренняя переменная
										{
											if (c.first == vnutriskobok)
											{
												vnutriskobokvp = true;                                                            
											}
										}
										if (vnutriskobokvp == true)
										{
											leftsomnoj = leftsomnoj + vnutrperem[vnutriskobok] + ")";
											string leftvalue = "{\"op\":\"*\",\"fO\":\"" + leftsomnoj+"\",";
											vnutrperem[leftcontent]=leftvalue;//добавление значения внутренней переменной
											//System.Console.WriteLine($"Внуренняя переменная {leftcontent} добавлена в коллекцию, ее значением является {leftvalue}");
											//System.Console.ReadKey();
										}
									}
									if ((IndexOf(rightrightcontent,"(") > 0))
									{
										int leftbracket = IndexOf(rightrightcontent,"(");
										string rightrightcontentname = rightrightcontent.substr(0, leftbracket);//Имя переменной справа от операции умножения
										//System.Console.WriteLine($"Имя переменной справа от операции умножения {rightrightcontentname}");
										//System.Console.ReadKey();
										string rightsomnoj = rightrightcontentname + "(";
										string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
										if (IndexOf(rightrightcontent,",") > 0)
										{
											vector<string> masindex = Split(vnutriskobok, ',' );
											for (string s : masindex)
											{
												//System.Console.WriteLine($"{s} - индекс правого сомножителя.");
												//System.Console.ReadKey();
												for (auto c : vnutrperem)//ищутся внутренние переменные для индексов правого сомножителя и определяются их значения
												{
													if (c.first == s)
													{
														rightsomnoj = rightsomnoj + vnutrperem[s] + ",";
														//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} правого сомножителя.");
														//System.Console.ReadKey();
													}
												}
											}
											rightsomnoj = rightsomnoj.substr(0, rightsomnoj.size() - 1) + ")";
											//System.Console.WriteLine($"{rightsomnoj} - значение правого сомножителя.");
											//System.Console.ReadKey();
										}
										if (IndexOf(rightrightcontent,",") < 0)
										{
											for (auto c : vnutrperem)//ищутся внутренние переменные для индексов правого сомножителя и определяются их значения
											{
												if (c.first == vnutriskobok)
												{
													rightsomnoj = rightsomnoj + vnutrperem[vnutriskobok];
													//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} правого сомножителя.");
													//System.Console.ReadKey();
												}
											}

											rightsomnoj = rightsomnoj + ")";
											//System.Console.WriteLine($"D - не находилась в коллекции, {rightsomnoj} - значение правого сомножителя.");
											//System.Console.ReadKey();
											bool outd = false;
											for (auto c : outdata)//ищутся внутренние переменные для индексов правого сомножителя и определяются их значения
											{
												if (c.first == rightsomnoj)
												{
													outd = true;
													rightsomnoj = outdata[rightsomnoj];
													//System.Console.WriteLine($"{rightsomnoj} - значение правого сомножителя, как выходной переменной X(j).");
													//System.Console.ReadKey();
												}
											}
										}
										//vnutrperem[leftcontent] = vnutrperem[leftcontent] + $"{rightsomnoj})";//изменение значения внутренней переменной
										//14.12.2021 НАЧАЛО ВСТАВКИ: проверяем, является ли правый сомножитель выходной переменной
										bool outdd = false;
										for (auto c : outdata)
										{
											if (c.first == rightsomnoj)
											{
												outdd = true;
												rightsomnoj = outdata[rightsomnoj];
												//System.Console.WriteLine($"{rightsomnoj} - значение правого сомножителя, как выходной переменной U(i,j).");
												//System.Console.ReadKey();
											}
										}
										//14.12.2021 КОНЕЦ ВСТАВКИ: проверяем, является ли правый сомножитель выходной переменной
										if (IndexOf(rightsomnoj,"{") >= 0)
										{
											vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":"+rightsomnoj + "}";
										}
										else 
										{ 
											vnutrperem[leftcontent] = vnutrperem[leftcontent] + "\"sO\":\""+rightsomnoj+"\"}";
										}
										//System.Console.WriteLine($"значением внуренней переменной {leftcontent} является {vnutrperem[leftcontent]}");
										//System.Console.ReadKey();
										//КОНЕЦ ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ВТОРОГО СОМНОЖИТЕЛЯ 
									}                                                
									string leftrightcontentvalue = " ";
									if ((IndexOf(leftrightcontent,"(") < 0))
									{
										bool leftrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, находится ли слева от операции умножения внутренняя переменная.
										{
											if (c.first == leftrightcontent) { leftrightcontentvp = true; };
										}
										if (leftrightcontentvp == true)
										{
											leftrightcontentvalue = vnutrperem[leftrightcontent];
											//System.Console.WriteLine($"Слева от операции умножения {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										bool leftrightcontentinp = false;
										for (string c : indata)// Проверяется, находится ли слева от операции умножения входная переменная.
										{
											if (c == leftrightcontent) { leftrightcontentinp = true; };
										}
										if (leftrightcontentinp == true)
										{
											leftrightcontentvalue = leftrightcontent;
											//System.Console.WriteLine($"Слева от операции умножения {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
										if ((leftrightcontent.substr(0, 1) == "0") || (leftrightcontent.substr(0, 1) == "1") || (leftrightcontent.substr(0, 1) == "2") || (leftrightcontent.substr(0, 1) == "3") || (leftrightcontent.substr(0, 1) == "4") || (leftrightcontent.substr(0, 1) == "5") || (leftrightcontent.substr(0, 1) == "6") ||
										   (leftrightcontent.substr(0, 1) == "7") || (leftrightcontent.substr(0, 1) == "8") || (leftrightcontent.substr(0, 1) == "9"))
										{
											//System.Console.WriteLine($"Значение первого сомножителя является числом {leftrightcontent}");
											//System.Console.ReadKey();
											leftrightcontentvalue = leftrightcontent;                                                    
										}
									}
									if ((IndexOf(rightrightcontent,"(") < 0))
									{
										//System.Console.WriteLine($"Имя переменной справа от операции умножения {rightrightcontent}");
										//System.Console.ReadKey();
										if ((rightrightcontent.substr(0, 1) == "0") || (rightrightcontent.substr(0, 1) == "1") || (rightrightcontent.substr(0, 1) == "2") || (rightrightcontent.substr(0, 1) == "3") || (rightrightcontent.substr(0, 1) == "4") || (rightrightcontent.substr(0, 1) == "5") || (rightrightcontent.substr(0, 1) == "6") ||
										   (rightrightcontent.substr(0, 1) == "7") || (rightrightcontent.substr(0, 1) == "8") || (rightrightcontent.substr(0, 1) == "9"))
										{
											//System.Console.WriteLine($"Значение второго сомножителя является числом {rightrightcontent}");
											//System.Console.ReadKey();
											string rightvalue = to_string((stoi(leftrightcontentvalue)) * (stoi(rightrightcontent)));
											vnutrperem[leftcontent]=rightvalue;
											//System.Console.WriteLine($"Внутренняя переменная {leftcontent} добавлена в коллекцию, ее значением является {rightvalue}");
											//System.Console.ReadKey();
										}
										bool rightrightcontentinp = false;
										for (string c : indata)// Проверяется, находится ли справа от операции умножения входная переменная.
										{
											if (c == rightrightcontent) { rightrightcontentinp = true; };
										}
										if (rightrightcontentinp == true)
										{
											string rightvalue = "{\"op\":\"*\",\"fO\":\"" + leftrightcontentvalue + "\",\"sO\":\"" + rightrightcontent+"\"}";
											vnutrperem[leftcontent]=rightvalue;
											//System.Console.WriteLine($"Добавлена внутренняя переменная {leftcontent}, в нее зпаписано значение {rightvalue}");
											//System.Console.ReadKey();
										}
										bool rightrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, находится ли справа от операции умножения внутренняя переменная.
										{
											if (c.first == rightrightcontent)
											{ rightrightcontentvp = true; };
										}
										if (rightrightcontentvp == true)
										{
											string rightvalue = "{\"op\":\"*\",\"fO\":\"" + leftrightcontentvalue + "\",\"sO\":" + vnutrperem[rightrightcontent] + "}";
											vnutrperem[leftcontent]=rightvalue;
											//System.Console.WriteLine($"Значение левого сомножителя {leftrightcontentvalue}");
											//System.Console.WriteLine($"Добавлена внутренняя переменная {leftcontent}, в нее зпаписано значение {rightvalue}");
											//System.Console.ReadKey();
										}
									}
								}
								if ((IndexOf(rightcontent,"/") > 0))
								{
									int locationoperation = IndexOf(rightcontent,"/");
									string leftrightcontent = rightcontent.substr(0, locationoperation);
									//System.Console.WriteLine($"Слева от операции деления {leftrightcontent}");
									//System.Console.ReadKey();
									string rightrightcontent = rightcontent.substr(locationoperation + 1, rightcontent.size() - locationoperation - 1);
									//System.Console.WriteLine($"Справа от операции деления {rightrightcontent}");
									//System.Console.ReadKey();
									string leftrightcontentvalue = " ";
									if ((IndexOf(leftrightcontent,"(") < 0))//делимое
									{
										bool leftrightcontentinp = false;
										for (string c : indata)// Проверяется, находится ли слева от операции деления входная переменная.
										{
											if (c == leftrightcontent) { leftrightcontentinp = true; };
										}
										if (leftrightcontentinp == true)
										{
											leftrightcontentvalue = leftrightcontent;
											//System.Console.WriteLine($"Слева от операции деления {leftrightcontent}, ее значение {leftrightcontentvalue}");
											//System.Console.ReadKey();
										}
									}
									if ((IndexOf(rightrightcontent,"(") < 0))//делитель
									{
										//System.Console.WriteLine($"Имя переменной справа от операции деления {rightrightcontent}");
										//System.Console.ReadKey();
										bool rightrightcontentvp = false;
										for (auto c : vnutrperem)// Проверяется, находится ли справа от операции деления внутренняя переменная.
										{
											if (c.first == rightrightcontent)
											{ rightrightcontentvp = true; };
										}
										if (rightrightcontentvp == true)
										{
											string rightvalue = "{\"op\":\"/\",\"fO\":\"" + leftrightcontentvalue + "\",\"sO\":" + vnutrperem[rightrightcontent] + "}";
											vnutrperem[leftcontent]=rightvalue;
											//System.Console.WriteLine($"Значение делимого {leftrightcontentvalue}");
											//System.Console.WriteLine($"Добавлена внутренняя переменная {leftcontent}, в нее зпаписано значение {rightvalue}");
											//System.Console.ReadKey();
										}
									}
								}
								if ((IndexOf(rightcontent,"abs(") == 0))
								{
									int locationoperation = IndexOf(rightcontent,"abc");
									string rightrightcontent = rightcontent.substr(locationoperation + 4, rightcontent.size() - locationoperation - 4);
									//System.Console.WriteLine($"Справа от операции abs {rightrightcontent}");
									//System.Console.ReadKey();                                                                                              
																					
									string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
									//System.Console.WriteLine($"Внутри скобок операции abs {vnutriskobok}");
									//System.Console.ReadKey();
									bool vnutriskobokvp = false;                                                    
									for (auto c : vnutrperem)//определяется значение внутри скобок операции abs
									{
											   if (c.first == vnutriskobok)
											   {
												  vnutriskobokvp = true;
											   }
									}
									if ((vnutriskobokvp == true))
									{
										string rightvalue = "{\"op\":\"abs\",\"od\":" + vnutrperem[vnutriskobok] + "}";
										vnutrperem[leftcontent]=rightvalue;//добавление значения внутренней переменной
										//System.Console.WriteLine($"Внуренняя переменная {leftcontent} добавлена в коллекцию, ее значением является {rightvalue}");
										//System.Console.ReadKey();
									}                                                                                        
								}
								if ((IndexOf(rightcontent,"sqrt(") == 0))
								{
									int locationoperation = IndexOf(rightcontent,"sqrt");
									string rightrightcontent = rightcontent.substr(locationoperation + 4, rightcontent.size() - locationoperation - 4);
									//System.Console.WriteLine($"Справа от операции sqrt {rightrightcontent}");
									//System.Console.ReadKey();                                                                                              

									string vnutriskobok = rightrightcontent.substr(IndexOf(rightrightcontent,"(") + 1, IndexOf(rightrightcontent,")") - IndexOf(rightrightcontent,"(") - 1);
									//System.Console.WriteLine($"Внутри скобок операции sqrt {vnutriskobok}");
									//System.Console.ReadKey();
									bool vnutriskobokvp = false;
									for (auto c : vnutrperem)//определяется значение внутри скобок операции abs
									{
										if (c.first == vnutriskobok)
										{
											vnutriskobokvp = true;
										}
									}
									if ((vnutriskobokvp == true))
									{
										string rightvalue = "{\"op\":\"sqrt\",\"od\":" + vnutrperem[vnutriskobok] + "}";
										vnutrperem[leftcontent]=rightvalue;//добавление значения внутренней переменной
										//System.Console.WriteLine($"Внуренняя переменная {leftcontent} добавлена в коллекцию, ее значением является {rightvalue}");
										//System.Console.ReadKey();
									}
								}
							}
						}                                    
					}
					for (auto ed : Edges)
					{
						if (ed.From == ver.Id)
						{
							nextb = ed.To;
						}
					}
				}
			if (ver.Type == 3)
			{
				//System.Console.WriteLine($"Тип 3: Обрабатываем блок {ver.Id} c Type {ver.Type}, Content: {ver.Content}");
				//System.Console.ReadKey();
				string content3 = ver.Content;
				string leftcontent3 = " ";
				string rightcontent3 = " ";
				string leftcontent3v = " ";
				string rightcontent3v = " ";
					if (IndexOf(content3,"<=") > 0)//для сравнения используется знак <=
					{
						int assignmentcharacter3 = IndexOf(content3,"<=");
						leftcontent3 = content3.substr(0, assignmentcharacter3);
						//System.Console.WriteLine($"Слева от <=: {leftcontent3}");
						//System.Console.ReadKey();
						rightcontent3 = content3.substr(assignmentcharacter3 + 2, content3.size() - assignmentcharacter3 - 2);
						//System.Console.WriteLine($"Справа от <=: {rightcontent3}");
						//System.Console.ReadKey();
					}
					if ((IndexOf(content3,"<") > 0) & (IndexOf(content3,"=") < 0))//для сравнения используется знак <
					{
						int assignmentcharacter3 = IndexOf(content3,"<");
						leftcontent3 = content3.substr(0, assignmentcharacter3);
						//System.Console.WriteLine($"Слева от <: {leftcontent3}");
						//System.Console.ReadKey();
						rightcontent3 = content3.substr(assignmentcharacter3 + 1, content3.size() - assignmentcharacter3 - 1);
						//System.Console.WriteLine($"Справа от <: {rightcontent3}");
						//System.Console.ReadKey();
					}
					if (IndexOf(content3,">=") > 0)//для сравнения используется знак <=
					{
						int assignmentcharacter3 = IndexOf(content3,">=");
						leftcontent3 = content3.substr(0, assignmentcharacter3);
						//System.Console.WriteLine($"Слева от <=: {leftcontent3}");
						//System.Console.ReadKey();
						rightcontent3 = content3.substr(assignmentcharacter3 + 2, content3.size() - assignmentcharacter3 - 2);
						//System.Console.WriteLine($"Справа от <=: {rightcontent3}");
						//System.Console.ReadKey();
					}
					if ((IndexOf(content3,">") > 0) & (IndexOf(content3,"=") < 0))//для сравнения используется знак >
					{
						int assignmentcharacter3 = IndexOf(content3,">");
						leftcontent3 = content3.substr(0, assignmentcharacter3);
						//System.Console.WriteLine($"Слева от >: {leftcontent3}");
						//System.Console.ReadKey();
						rightcontent3 = content3.substr(assignmentcharacter3 + 1, content3.size() - assignmentcharacter3 - 1);
						//System.Console.WriteLine($"Справа от >: {rightcontent3}");
						//System.Console.ReadKey();
					}
					if (IndexOf(content3,"==") > 0)//для сравнения используется знак ==
					{
						int assignmentcharacter3 = IndexOf(content3,"=");
						leftcontent3 = content3.substr(0, assignmentcharacter3);
						//System.Console.WriteLine($"Слева от ==: {leftcontent3}");
						//System.Console.ReadKey();
						rightcontent3 = content3.substr(assignmentcharacter3 + 2, content3.size() - assignmentcharacter3 - 2);
						//System.Console.WriteLine($"Справа от ==: {rightcontent3}");
						//System.Console.ReadKey();
					}
					if (IndexOf(content3,"!=") > 0)//для сравнения используется знак !=
					{
						int assignmentcharacter3 = IndexOf(content3,"!=");
						leftcontent3 = content3.substr(0, assignmentcharacter3);
						//System.Console.WriteLine($"Слева от !=: {leftcontent3}");
						//System.Console.ReadKey();
						rightcontent3 = content3.substr(assignmentcharacter3 + 2, content3.size() - assignmentcharacter3 - 2);
						//System.Console.WriteLine($"Справа от !=: {rightcontent3}");
						//System.Console.ReadKey();
					}

				bool vnupl = false; //проверяем: слева от знака сравнения внутренняя переменная без индексов?
				for (auto c : vnutrperem)
				{
					if (c.first == leftcontent3)
					{
						vnupl = true;
						leftcontent3v = vnutrperem[leftcontent3];
						//System.Console.WriteLine($"Слева от знака сравнения внутренняя переменная {leftcontent3}, ее значение: {vnutrperem[leftcontent3]}");
						//System.Console.ReadKey();
					}
				}
					bool vnupli = false;
					bool outpli = false;
					bool inplii = false;
					if (IndexOf(leftcontent3,"(") > 0) //НАЧАЛО ВСТАВКИ ДЛЯ ОПРЕДЕЛЕНИЯ ПЕРЕМЕННОЙ СЛЕВА ОТ ЗНАКА СРАВНЕНИЯ
					//проверяем: слева от знака сравнения внутренняя переменная с несколькими индексами?
					{
						int leftbracket = IndexOf(leftcontent3,"(");
						string leftcontentname = leftcontent3.substr(0, leftbracket);//имя переменной слева от скобки (
						//System.Console.WriteLine($"Имя переменной слева от знака сравнения {leftcontentname}");
						//System.Console.ReadKey();
						string leftotzsr = leftcontentname + "(";
						string vnutriskobok = leftcontent3.substr(IndexOf(leftcontent3,"(") + 1, IndexOf(leftcontent3,")") - IndexOf(leftcontent3,"(") - 1);
						//29.11.2021 НАЧАЛО ВСТАВКИ ДЛЯ ВНУТРЕННЕЙ ПЕРЕМЕННОЙ С ОДНИМ ИНДЕКСОМ, А ТАКЖЕ ВХОДНОЙ ПЕРЕМЕННОЙ С ОДНИМ ИНДЕКСОМ СЛЕВА ОТ ЗНАКА СРАВНЕНИЯ
						if (IndexOf(leftcontent3,",") < 0)//один индекс
						{
							for (auto c : vnutrperem)//ищется внутренняя переменная для индекса переменной слева от знака сравнения и определяется ее значение
							{
								if (c.first == vnutriskobok)
								{
									leftotzsr = leftotzsr + vnutrperem[vnutriskobok] + ")";
									//System.Console.WriteLine($"{vnutrperem[vnutriskobok]} - значение индекса {vnutriskobok} переменной слева от знака сравнения.");
									//System.Console.ReadKey();
								}
							}
							vnupli = false; //проверяем: слева от знака сравнения внутренняя переменная с одним индексом?
							for (auto c : vnutrperem)
							{
								if (c.first == leftotzsr)
								{
									vnupli = true;
									leftcontent3v = vnutrperem[leftotzsr];
									//System.Console.WriteLine($"Значение внутренней переменной с одним индексом слева от знака сравнения: {vnutrperem[leftotzsr]}");
									//System.Console.ReadKey();
								}
							}
							outpli = false; //проверяем: слева от знака сравнения выходная переменная с одним индексом?
							for (auto c : outdata)
							{
								if (c.first == leftotzsr)
								{
									outpli = true;
									leftcontent3v = outdata[leftotzsr];
									//System.Console.WriteLine($"Значение выходной переменной с одним индексом слева от знака сравнения: {outdata[leftotzsr]}");
									//System.Console.ReadKey();
								}
							}
						}
						//29.11.2021 КОНЕЦ ВСТАВКИ ДЛЯ ВНУТРЕННЕЙ ПЕРЕМЕННОЙ С ОДНИМ ИНДЕКСОМ, А ТАКЖЕ ВХОДНОЙ ПЕРЕМЕННОЙ С ОДНИМ ИНДЕКСОМ СЛЕВА ОТ ЗНАКА СРАВНЕНИЯ                                    
						if (IndexOf(leftcontent3,",") > 0)//несколько индексов
						{
							vector<string> masindex = Split(vnutriskobok, ',');
							for (string s : masindex)
							{
								//System.Console.WriteLine($"{s} - индекс переменной слева от знака сравнения.");
								//System.Console.ReadKey();
								for (auto c : vnutrperem)//ищутся внутренние переменные для индексов переменной слева от знака сравнения и определяются их значения
								{
									if (c.first == s)
									{
										leftotzsr = leftotzsr + vnutrperem[s] + ",";
										//System.Console.WriteLine($"{vnutrperem[s]} - значение индекса {s} переменной слева от знака сравнения.");
										//System.Console.ReadKey();
									}
								}
							}
							leftotzsr = leftotzsr.substr(0, leftotzsr.size() - 1) + ")";
							bool vnuplii = false; //проверяем: слева от знака сравнения внутренняя переменная с несколькими индексами?
							for (auto c : vnutrperem)
							{
								if (c.first == leftotzsr)
								{
									vnuplii = true;
									leftcontent3v = vnutrperem[leftotzsr];
									//System.Console.WriteLine($"Значение переменной слева от знака сравнения: {vnutrperem[leftotzsr]}");
									//System.Console.ReadKey();
								}
							}
							//29.11.2021 НАЧАЛО ВСТАВКИ ДЛЯ ВХОДНОЙ ПЕРЕМЕННОЙ С НЕСКОЛЬКИМИ ИНДЕКСАМИ СЛЕВА ОТ ЗНАКА СРАВНЕНИЯ
							inplii = false; //проверяем: слева от знака сравнения входная переменная с несколькими индексами?
							for (string c : indata)
							{
								if (c == leftotzsr)
								{
									inplii = true;
									leftcontent3v = leftotzsr;
									//System.Console.WriteLine($"Значение входной переменной с несколькими индексами слева от знака сравнения: {leftcontent3v}");
									//System.Console.ReadKey();
								}
							}
						}
					}
				bool outpl = false; //проверяем: слева от знака сравнения выходная переменная без индексов?
				for (auto c : outdata)
				{
					if (c.first == leftcontent3)
					{
						outpl = true;
						leftcontent3v = outdata[leftcontent3];
						//System.Console.WriteLine($"Значение переменной слева от знака сравнения: {outdata[leftcontent3]}");
						//System.Console.ReadKey();
					}
				}
				bool parrr = false; //проверяем: справа от знака сравнения параметр размерности?
				for (auto c : paramrazm)
				{
					if (c.first == rightcontent3)
					{
						parrr = true;
						//System.Console.WriteLine($"Значение переменной справа от знака сравнения: {paramrazm[rightcontent3]}");
						//System.Console.ReadKey();
					}
				}
					bool inpri = false;
					bool vnupri = false;
					bool outpri = false;
					if ((IndexOf(rightcontent3,"(") > 0) & (IndexOf(rightcontent3,",") < 0)) //анализируем правую часть в условии, когда есть скобки, но в них один индекс
					{
						string rightc = rightcontent3.substr(IndexOf(rightcontent3,"(") + 1, IndexOf(rightcontent3,")") - IndexOf(rightcontent3,"(") - 1);
						//System.Console.WriteLine($"В скобках справа от знака сравнения в {rightcontent3} переменная {rightc}");
						//System.Console.ReadKey();                                     
						for (string c : indata)//проверяем: справа от знака сравнения входная переменная с одним индексом?
						{
							if (c == (rightcontent3.substr(0, IndexOf(rightcontent3,"(")) + "(" + vnutrperem[rightc] + ")"))
							{
								inpri = true;
								rightcontent3v = rightcontent3.substr(0, IndexOf(rightcontent3,"(")) + "(" + vnutrperem[rightc] + ")";
								//System.Console.WriteLine($"В условии справа от знака сравнения {rightcontent3v}");
								//System.Console.ReadKey();
							}
						}                                   
						for (auto c : vnutrperem)//проверяем: справа от знака сравнения внутренняя переменная c индексом?
						{
							if (c.first == (rightcontent3.substr(0, IndexOf(rightcontent3,"(")) + "(" + vnutrperem[rightc] + ")"))
							{
								vnupri = true;
								rightcontent3v = vnutrperem[rightcontent3.substr(0, IndexOf(rightcontent3,"(")) + "(" + vnutrperem[rightc] + ")"];
								//System.Console.WriteLine($"В условии справа от знака сравнения {rightcontent3.substr(0, rightcontent3IndexOf(,"(")) + "(" + vnutrperem[rightc] + ")"}, ее значение {rightcontent3v}");
								//System.Console.ReadKey();
							}
						}
						//05.12.2021 НАЧАЛО: ВСТАВКА ДЛЯ ОПРЕДЕЛЕНИЯ, НАХОДИТСЯ ЛИ СПРАВА ОТ ЗНАКА СРАВНЕНИЯ ВЫХОДНАЯ ПЕРЕМЕННАЯ С ОДНИМ ИНДЕКСОМ                                    
						for (auto c : outdata)//проверяем: справа от знака сравнения выходная переменная c индексом?
						{
							if (c.first == (rightcontent3.substr(0, IndexOf(rightcontent3,"(")) + "(" + vnutrperem[rightc] + ")"))
							{
								outpri = true;
								rightcontent3v = outdata[rightcontent3.substr(0, IndexOf(rightcontent3,"(")) + "(" + vnutrperem[rightc] + ")"];
								//System.Console.WriteLine($"В условии справа от знака сравнения выходная переменная {rightcontent3.substr(0, rightcontent3IndexOf(,"(")) + "(" + vnutrperem[rightc] + ")"}, ее значение {rightcontent3v}");
								//System.Console.ReadKey();
							}
						}
						//05.12.2021 КОНЕЦ: ВСТАВКА ДЛЯ ОПРЕДЕЛЕНИЯ, НАХОДИТСЯ ЛИ СПРАВА ОТ ЗНАКА СРАВНЕНИЯ ВЫХОДНАЯ ПЕРЕМЕННАЯ С ОДНИМ ИНДЕКСОМ
					}
					bool vnupr = false; //проверяем: справа от знака сравнения внутренняя переменная без индекса?
					for (auto c : vnutrperem)
					{
						if (c.first == rightcontent3)
						{
							vnupr = true;
							rightcontent3v = vnutrperem[rightcontent3];
							//System.Console.WriteLine($"Значение переменной справа от знака сравнения: {vnutrperem[rightcontent3]}");
							//System.Console.ReadKey();
						}
					}
					/*bool outpr = false; //проверяем: справа от знака сравнения выходная переменная без индекса?
					for (string c : outdata)
					{
						if (c == rightcontent3)
						{
							outpr = true;
							System.Console.WriteLine($"Значение переменной справа от знака сравнения: {outdata[rightcontent3]}");
							System.Console.ReadKey();
						}
					}*/
					bool inpr = false;
					for (string c : indata)//проверяем: справа от знака сравнения входная переменная без индекса?
					{
						//System.Console.WriteLine($"Входная переменная в коллекции indata {c}");
						//System.Console.ReadKey();                                   
						if (c == rightcontent3)
						{
							inpr = true;
							rightcontent3v = rightcontent3;
							//System.Console.WriteLine($"Значение переменной справа от знака сравнения: {rightcontent3v}");
							//System.Console.ReadKey();
						}
					}
					bool numberr = false; //Справа от знака сравнения число
					if ((rightcontent3.substr(0, 1) == "0") || (rightcontent3.substr(0, 1) == "1") || (rightcontent3.substr(0, 1) == "2") || (rightcontent3.substr(0, 1) == "3") || (rightcontent3.substr(0, 1) == "4") || (rightcontent3.substr(0, 1) == "5") || (rightcontent3.substr(0, 1) == "6") ||
						(rightcontent3.substr(0, 1) == "7") || (rightcontent3.substr(0, 1) == "8") || (rightcontent3.substr(0, 1) == "9"))
					{
						numberr = true;
						rightcontent3v = rightcontent3;
						//System.Console.WriteLine($"Значение переменной справа от знака сравнения: {rightcontent3v}");
						//System.Console.ReadKey();
					}
					bool uslovie = false;
					if ((vnupl == true) & (parrr == true) & (IndexOf(content3,"<=") > 0))
					{
						if ((stoi(vnutrperem[leftcontent3]) <= paramrazm[rightcontent3])) uslovie = true;
					}
					if ((vnupl == true) & (parrr == true) & (IndexOf(content3,"<") > 0))
					{
						if (IndexOf(content3,"=") < 0)
						{
							if ((stoi(vnutrperem[leftcontent3]) < paramrazm[rightcontent3])) uslovie = true;
							//System.Console.WriteLine($"Слева {vnutrperem[leftcontent3]}, справа {vnutrperem[rightcontent3]}, условие равно {uslovie}");
							//System.Console.WriteLine($"leftcontent3: {leftcontent3}; vnutrperem[leftcontent3]: {vnutrperem[leftcontent3]}");
							//System.Console.ReadKey();
						}
					}
					if ((vnupl == true) & (vnupr == true) & (IndexOf(content3,"<") > 0))
					{
						if (IndexOf(content3,"=") < 0)
						{
							if ((stoi(vnutrperem[leftcontent3]) < stoi(vnutrperem[rightcontent3]))) uslovie = true;
							//System.Console.WriteLine($"Слева {vnutrperem[leftcontent3]}, справа {vnutrperem[rightcontent3]}, условие равно {uslovie}");
							//System.Console.ReadKey();
						}
					}
					if ((vnupl == true) & (vnupr == true) & (IndexOf(content3,">") > 0))
					{
						if (IndexOf(content3,"=") < 0)
						{
							if ((stoi(vnutrperem[leftcontent3]) > stoi(vnutrperem[rightcontent3]))) uslovie = true;
							//System.Console.WriteLine($"Слева {vnutrperem[leftcontent3]}, справа {vnutrperem[rightcontent3]}, условие равно {uslovie}");
							//System.Console.ReadKey();
						}
					}
					if ((vnupl == true) & (vnupr == true) & (IndexOf(content3,">=") > 0))
					{
						if ((stoi(vnutrperem[leftcontent3]) >= stoi(vnutrperem[rightcontent3]))) uslovie = true;
						//System.Console.WriteLine($"Слева {vnutrperem[leftcontent3]}, справа {vnutrperem[rightcontent3]}, условие равно {uslovie}");
						//System.Console.ReadKey();
					}
					if ((vnupl == true) & (vnupr == true) & (IndexOf(content3,"!=") > 0))
					{
						if ((stoi(vnutrperem[leftcontent3]) != stoi(vnutrperem[rightcontent3]))) uslovie = true;
						//System.Console.WriteLine($"Слева {vnutrperem[leftcontent3]}, справа {vnutrperem[rightcontent3]}, условие равно {uslovie}");
						//System.Console.ReadKey();
					}
					if ((vnupl == true) & (vnupri == true) & (IndexOf(content3,"==") > 0))
					{
						
						if ((stoi(vnutrperem[leftcontent3]) == stoi(rightcontent3v))) uslovie = true;
						//System.Console.WriteLine($"Слева {leftcontent3} со значением {vnutrperem[leftcontent3]}, справа {rightcontent3} со значением {rightcontent3v}, условие равно {uslovie}");
						//System.Console.ReadKey();                                                                        
					}
					if ((vnupl == true) & (numberr == true) & (IndexOf(content3,"<") > 0))
					{
						if (IndexOf(content3,"=") < 0)
						{
							if ((stoi(vnutrperem[leftcontent3]) < stoi(rightcontent3))) uslovie = true;
							//System.Console.WriteLine($"Слева {vnutrperem[leftcontent3]}, справа {rightcontent3}, условие равно {uslovie}");
							//System.Console.ReadKey();
						}
					}
					//01.12.2021 НАЧАЛО ВСТАВКИ ДЛЯ ОБРАБОТКИ БЛОКА ТИПА 3: СЛЕВА ВНУТРЕННЯЯ ПЕРЕМЕННАЯ С ОДНИМ ИНДЕКСОМ, СПРАВА ЧИСЛО, ОПЕРАЦИЯ СРАВНЕНИЯ "=="
					if ((vnupli == true) & (numberr == true) & (IndexOf(content3,"==") > 0))
					{
							if (stoi(leftcontent3v) == stoi(rightcontent3v)) uslovie = true;
							//System.Console.WriteLine($"Слева {leftcontent3v}, справа {rightcontent3}, условие равно {uslovie}");
							//System.Console.ReadKey();                                 
					}
					//01.12.2021 КОНЕЦ ВСТАВКИ ДЛЯ ОБРАБОТКИ БЛОКА ТИПА 3: СЛЕВА ВНУТРЕННЯЯ ПЕРЕМЕННАЯ С ОДНИМ ИНДЕКСОМ, СПРАВА ЧИСЛО, ОПЕРАЦИЯ СРАВНЕНИЯ "=="
					if ((vnupl == true) & (rightcontent3 == "iterations") & (IndexOf(content3,"<=") > 0))
					{
						//System.Console.WriteLine($"Слева в блоке с iterations {stoi(vnutrperem[leftcontent3])}, справа {iterations}");
						//System.Console.ReadKey();
						if ((stoi(vnutrperem[leftcontent3]) <= iterations)) uslovie = true;
						//System.Console.WriteLine($"uslovie для блока с iterations равно {uslovie}");
						//System.Console.ReadKey();
					}
					bool lc3v = false;
					for (string c : indata)//проверяем: содержится ли входная переменная в leftcontent3v?
					{
						//System.Console.WriteLine("Вошли в цикл проверки вхождения входной переменной в левую часть оператора сравнения");
						//System.Console.WriteLine($"с: {c}, leftcontent3v: {leftcontent3v}");
						//System.Console.ReadKey();
						if (IndexOf(leftcontent3v,c) >= 0)
						{                                        
							lc3v = true;                                        
							//System.Console.WriteLine($"входная переменная {c} содержится в {leftcontent3v}");
							//System.Console.ReadKey();
						}
					}
					bool rc3v = false;
					for (string c : indata)//проверяем: содержится ли входная переменная в rightcontent3v?
					{
						//System.Console.WriteLine("Вошли в цикл проверки вхождения входной переменной в правую часть оператора сравнения");
						//System.Console.WriteLine($"с: {c}, rightcontent3v: {rightcontent3v}");
						//System.Console.ReadKey();
						if (IndexOf(rightcontent3v,c) >= 0)
						{
							rc3v = true;
							//System.Console.WriteLine($"входная переменная {c} содержится в {rightcontent3v}");
							//System.Console.ReadKey();
						}
					}
					if (((lc3v == true) || (rc3v == true)) & (rightcontent3 != "iterations"))//для логического Q-терма
					{
						if (way.size() <= numberinway)
						{
						   numberinway = numberinway + 1;
							way[numberinway] = to_string(ver.Id) + "," + "1";
							uslovie = true;
						}
						if (numberinway < way.size())
						{
							numberinway = numberinway + 1;
							if (IndexOf(way[numberinway],",1") > 0) uslovie = true;
							if (IndexOf(way[numberinway],",0") > 0) uslovie = false;
						}
						if ((IndexOf(content3,"<") > 0) & (IndexOf(content3,"=") < 0) & (uslovie == true) & (rightcontent3 == "e"))
						{

							//System.Console.WriteLine($"Значение переменной logicalqterm: {logicalqterm}");
							//System.Console.ReadKey();
							//System.Console.WriteLine("Условие сходимости выполнено");
							//System.Console.ReadKey();
							if (logicalqterm == " ") { logicalqterm = "{\"op\":\"<\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}"; }
							else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"<\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}}"; }
							//System.Console.WriteLine($"Значение переменной logicalqterm: {logicalqterm}");
							//System.Console.ReadKey();
						}
						if ((IndexOf(content3,"<") > 0) & (IndexOf(content3,"=") < 0) & (uslovie == true) & (rightcontent3 != "e"))
						{
							//System.Console.WriteLine($"Обрабатывается условие {content3} в случае TRUE");
							//System.Console.ReadKey();
							//06.12.2021 НАЧАЛО ВСТАВКИ ДЛЯ УЧЕТА, ЧТО ЗНАЧЕНИЯ ОБЕИХ ЧАСТЕЙ УСЛОВИЯ МОГУТ ИМЕТЬ СКОБКИ { и }
							if ((IndexOf(leftcontent3v,"{") < 0) & (IndexOf(rightcontent3v,"{") < 0))
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"<\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"<\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
							if ((IndexOf(leftcontent3v,"{") < 0) & (IndexOf(rightcontent3v,"{") >= 0))
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"<\",\"fO\":\"" + leftcontent3v + "\",\"sO\":" + rightcontent3v + "}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"<\",\"fO\":\"" + leftcontent3v + "\",\"sO\":" + rightcontent3v + "}}"; }
							}
							if ((IndexOf(leftcontent3v,"{") >= 0) & (IndexOf(rightcontent3v,"{") < 0))
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"<\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"<\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
							if ((IndexOf(leftcontent3v,"{") >= 0) & (IndexOf(rightcontent3v,"{") >= 0))
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"<\",\"fO\":" + leftcontent3v + ",\"sO\":" + rightcontent3v + "}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"<\",\"fO\":" + leftcontent3v + ",\"sO\":" + rightcontent3v + "}}"; }
							}
						}
						if ((IndexOf(content3,"==") > 0) & (uslovie == true) & (rightcontent3 != "e"))
						{
							//if (logicalqterm == " ") { logicalqterm = "<" + "(" + leftcontent3v + "," + rightcontent3v + ")"; }
							//else { logicalqterm = "&" + "(" + logicalqterm + "," + "<" + "(" + leftcontent3v + "," + rightcontent3v + "))"; }
							//string logicalqterm1;
							if (IndexOf(leftcontent3v,"{") < 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"==\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"==\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}}"; }                                          
							}
							if (IndexOf(leftcontent3v,"{") >= 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"==\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"==\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
							//System.Console.WriteLine($"Значение переменной logicalqterm: {logicalqterm}");
							//System.Console.ReadKey();
						}
						if ((IndexOf(content3,">") > 0) & (IndexOf(content3,"=") < 0) & (uslovie == true) & (rightcontent3 != "e"))
						{
							//if (logicalqterm == " ") { logicalqterm = "<" + "(" + leftcontent3v + "," + rightcontent3v + ")"; }
							//else { logicalqterm = "&" + "(" + logicalqterm + "," + "<" + "(" + leftcontent3v + "," + rightcontent3v + "))"; }
							//string logicalqterm1;
							if (IndexOf(leftcontent3v,"{") < 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\">\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\">\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
							if (IndexOf(leftcontent3v,"{") >= 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\">\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\">\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
						}
						if ((IndexOf(content3,"!") > 0) & (IndexOf(content3,"=") > 0) & (uslovie == true) & (rightcontent3 != "e"))
						{
							if (IndexOf(leftcontent3v,"{") < 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"!=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"!=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}}"; }
								//System.Console.WriteLine($"Значение переменной logicalqterm: {logicalqterm}");
								//System.Console.ReadKey();
							}
							if (IndexOf(leftcontent3v,"{") >= 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"!=\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"!=\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}}"; }
								//System.Console.WriteLine($"Значение переменной logicalqterm: {logicalqterm}");
								//System.Console.ReadKey();
							}
						}
						if ((IndexOf(content3,"<") > 0) & (IndexOf(content3,"=") < 0) & (uslovie == false) & (rightcontent3 != "e"))
						{
							if ((IndexOf(leftcontent3v,"{") < 0) & (IndexOf(rightcontent3v,"{") < 0))
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\">=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\">=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
							if ((IndexOf(leftcontent3v,"{") < 0) & (IndexOf(rightcontent3v,"{") >= 0))
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\">=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":" + rightcontent3v + "}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\">=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":" + rightcontent3v + "}}"; }
							}
							if ((IndexOf(leftcontent3v,"{") >= 0) & (IndexOf(rightcontent3v,"{") < 0))
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\">=\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\">=\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
							if ((IndexOf(leftcontent3v,"{") >= 0) & (IndexOf(rightcontent3v,"{") >= 0))
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\">=\",\"fO\":" + leftcontent3v + ",\"sO\":" + rightcontent3v + "}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\">=\",\"fO\":" + leftcontent3v + ",\"sO\":" + rightcontent3v + "}}"; }
							}
						}
						if ((IndexOf(content3,"==") > 0) & (uslovie == false) & (rightcontent3 != "e"))
						{
							//if (logicalqterm == " ") { logicalqterm = ">=" + "(" + leftcontent3v + "," + rightcontent3v + ")"; }
							//else { logicalqterm = "&" + "(" + logicalqterm + "," + ">=" + "(" + leftcontent3v + "," + rightcontent3v + "))"; }
							//string logicalqterm2;
							if (IndexOf(leftcontent3v,"{") < 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"!=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"!=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
							if (IndexOf(leftcontent3v,"{") >= 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"!=\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"!=\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
						}
						if ((IndexOf(content3,">") > 0) & (IndexOf(content3,"=") < 0) & (uslovie == false) & (rightcontent3 != "e"))
						{
							if (IndexOf(leftcontent3v,"{") < 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"<=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"<=\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
							if (IndexOf(leftcontent3v,"{") >= 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"<=\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"<=\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
						}
						if ((IndexOf(content3,"!") > 0) & (IndexOf(content3,"=") > 0) & (uslovie == false) & (rightcontent3 != "e"))
						{
							if (IndexOf(leftcontent3v,"{") < 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"==\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"==\",\"fO\":\"" + leftcontent3v + "\",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
							if (IndexOf(leftcontent3v,"{") >= 0)
							{
								if (logicalqterm == " ") { logicalqterm = "{\"op\":\"==\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}"; }
								else { logicalqterm = "{\"op\":\"&\",\"fO\":" + logicalqterm + ",\"sO\":" + "{\"op\":\"==\",\"fO\":" + leftcontent3v + ",\"sO\":\"" + rightcontent3v + "\"}}"; }
							}
					   }
					}
				for (auto ed : Edges)
				{
					if ((ed.From == ver.Id) & (ed.Type == 1) & (uslovie == true))
					{
						nextb = ed.To;
					}
					if ((ed.From == ver.Id) & (ed.Type == 0) & (uslovie == false))
					{
						nextb = ed.To;
					}
				}
			}
			if (!((ver.Type == 2) || (ver.Type == 3)))
			{                            
				for (auto ed : Edges)
				{
					if (ed.From == ver.Id)
					{
						nextb = ed.To;
					}
				}
			}
			}
		}
	}
	if (IndexOf(vnutrperem["empout"],"1") >= 0) //Данные для Q-детерминанта будут выведены
	{
		if ((iterations == 0))
		{
			for (auto c : outdata)
			{
				cout << c.first + "=" + logicalqterm + ";" + c.second << endl;
			}
		}
		if ((iterations > 0) & (IndexOf(way[numberinway],",1") > 0))
		{
			for (auto c : outdata)
			{
				cout << c.first + "=" + logicalqterm + ";" + c.second << endl;
			}
		}
	}
	}
}

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	
	ReadFile();
	for (int i=argc-1; i>0; i--)
	{
		inparams.push_back(stoi(argv[i]));
	}
	
	GetDim();//извлечение из блок-схемы параметров размерности
	GetInVal();//извлечение из блок-схемы входных переменных
	GetOutVal();//извлечение из блок-схемы выходных переменных
	
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	int k = 0;//номер прохода по блок-схеме
	bool proxod = true;
	while (proxod)
	{
		k=k+1;
		if (k % size == rank)
		{
			PassBS();
		}
		else
		{
			Skip(argc, stoi(argv[1]), k);
		}
		if (numberinway > 0)
		{
			while (IndexOf(way[numberinway],",0") > 0)
			{
				way.erase(numberinway);
				numberinway = numberinway - 1;
				if (numberinway == 0) break;
			}
		}
		if (numberinway > 0)
		{
			if (IndexOf(way[numberinway],",1") > 0)
			{
				way[numberinway] = way[numberinway].substr(0, IndexOf(way[numberinway],",1") + 1) + "0";
			}
		}
		numberinway = 0;
		if (way.size() == 0) proxod = false;
	}
	
	MPI_Finalize();
	return 0;
}