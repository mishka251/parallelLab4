#include<mpi.h>
#include<iostream>
#include<fstream>
#include<time.h>
#include<algorithm>
#include<chrono>
#include<locale.h>
#include<Windows.h>
#include<string>
#include<math.h>

using namespace std;


double* arr;//общий массив
double *my_arr;//массив одного потока
int N;//размер массива
int R;//рандом числа [1, R]
int M;//магическое число
int my_n;//размер массива на одном процессе
int stop = false;//остановка. Если какой-то процесс найдет M то запишет сюда тру и отрпавит всем
int* ind_arr;//массив для индексов от каждого потока

void init();//инициализация(заполнение массивов)
template<typename T>
void printArr(T*a, int n, ofstream& of);//вывод массива в файл


void send(int rank, int size)//отправка данных каждому процессу
{
	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&R, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&M, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	my_n = N / size;//элементов в каждом процессе
	if (N%size != 0)//если не поровну
	{
		my_n++;
		if (rank == 0)
		{
			//добавляем размер до нужного
			arr = (double*)realloc(arr, sizeof(double)*my_n*size);
			//добавляем в арр  чтобы делилось
			for (int i = N; i < my_n*size; i++)
				arr[i] = -1;
		}
	}

	//массив в процессе
	my_arr = new double[my_n];
	//рассылаем с 0 на остальные
	MPI_Scatter(arr, my_n, MPI_DOUBLE, my_arr, my_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	/*рассылка данных из 0 процесса всем остальным*/
}

//работа процесса
void procAction(int rank, int size)
{
	int i;
	int next = (rank + 1) % size;
	int prev = (rank + size - 1) % size;
	MPI_Request recive;

	int stop_buf = 0;
	int TAG = 1;
	MPI_Irecv(&stop_buf, 1, MPI_INT, prev, TAG, MPI_COMM_WORLD, &recive);

	for (i = 0; i < my_n; i++)
	{
		int flag = 0;
		MPI_Status recived;
		MPI_Test(&recive, &flag, &recived);
		if (flag)
		{
			stop = true;//т.к. отправляем только тру
			MPI_Send(&stop, 1, MPI_INT, next, TAG, MPI_COMM_WORLD);
		}

		if (my_arr[i] == M)
		{
			//STOP
			stop = true;
			MPI_Send(&stop, 1, MPI_INT, next, TAG, MPI_COMM_WORLD);
		}
		if (stop)
		{
			break;
		}
		my_arr[i] = sin(my_arr[i] - M)*exp(my_arr[i]) - log(abs(cos(my_arr[i])));
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (!stop)//прошли весь
		MPI_Gather(my_arr, my_n, MPI_DOUBLE, arr, my_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	else//нашли
		MPI_Gather(&i, 1, MPI_INT, ind_arr, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

//вся лаба тут
void laba(int argc, char **argv)
{
	//номер процесса, кол-во
	int rank, size;

	MPI_Init(&argc, &argv);
	//получили
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//на 0 процессе создали массивы
	if (rank == 0)
	{
		ofstream out("startArray.txt");
		init();
		out << "Исходный массив" << endl;
		printArr(arr, N, out);
		out.close();
	}

	ind_arr = new int[size];
	MPI_Barrier(MPI_COMM_WORLD);
	send(rank, size);
	MPI_Barrier(MPI_COMM_WORLD);
	procAction(rank, size);
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	if (rank == 0)
	{
		ofstream out("output.txt");
		if (stop)
		{
			out << "Остановлен" << std::endl;
			out << "Процессы остановились на элементах" << std::endl;
			printArr(ind_arr, size, out);
		}
		else
		{
			out << "Все пройдено, М не найдено. Измененный массив" << std::endl;
			printArr(arr, N, out);
		}
		out.close();
	}
}

int main(int argc, char **argv)
{
	laba(argc, argv);
	return 0;
}




void init()
{
	cout << "N=";
	cin >> N;
	cout << "R=";
	cin >> R;
	cout << "M=";
	cin >> M;
	arr = new double[N];
	srand(unsigned(time(0)));
	for (int i = 0; i < N; i++)
		arr[i] = 1 + rand() % R;
}

template<typename T>
void printArr(T*a, int n, ofstream& of)
{
	const int elementInOneLine = 10;
	for (int i = 0; i < n; i++)
	{
		of << a[i] << " ";
		if (i%elementInOneLine == elementInOneLine - 1)
			of << endl;
	}
	of << endl;
	of.close();
}
