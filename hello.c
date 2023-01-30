int buf[2][100];

// sort [l, r)

int main()
{
    int n = getarray(buf[0]);
    merge_sort(0, n);
    putarray(n, buf[0]);
    return 0;
}