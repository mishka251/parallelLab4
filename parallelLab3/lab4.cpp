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


double* arr;//����� ������
double *my_arr;//������ ������ ������
int N;//������ �������
int R;//������ ����� [1, R]
int M;//���������� �����
int my_n;//������ ������� �� ����� ��������
int stop = false;//���������. ���� �����-�� ������� ������ M �� ������� ���� ��� � �������� ����
int* ind_arr;//������ ��� �������� �� ������� ������

void init();//�������������(���������� ��������)
template<typename T>
void printArr(T*a, int n, ofstream& of);//����� ������� � ����


void send(int rank, int size)//�������� ������ ������� ��������
{
	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&R, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&M, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	my_n = N / size;//��������� � ������ ��������
	if (N%size != 0)//���� �� �������
	{
		my_n++;
		if (rank == 0)
		{
			//��������� ������ �� �������
			arr = (double*)realloc(arr, sizeof(double)*my_n*size);
			//��������� � ���  ����� ��������
			for (int i = N; i < my_n*size; i++)
				arr[i] = -1;
		}
	}

	//������ � ��������
	my_arr = new double[my_n];
	//��������� � 0 �� ���������
	MPI_Scatter(arr, my_n, MPI_DOUBLE, my_arr, my_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	/*�������� ������ �� 0 �������� ���� ���������*/
}

//������ ��������
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
			stop = true;//�.�. ���������� ������ ���
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

	if (!stop)//������ ����
		MPI_Gather(my_arr, my_n, MPI_DOUBLE, arr, my_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	else//�����
		MPI_Gather(&i, 1, MPI_INT, ind_arr, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

//��� ���� ���
void laba(int argc, char **argv)
{
	//����� ��������, ���-��
	int rank, size;

	MPI_Init(&argc, &argv);
	//��������
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//�� 0 �������� ������� �������
	if (rank == 0)
	{
		ofstream out("startArray.txt");
		init();
		out << "�������� ������" << endl;
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
			out << "����������" << std::endl;
			out << "�������� ������������ �� ���������" << std::endl;
			printArr(ind_arr, size, out);
		}
		else
		{
			out << "��� ��������, � �� �������. ���������� ������" << std::endl;
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
