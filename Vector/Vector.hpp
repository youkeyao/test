#ifndef Vector_hpp
#define Vector_hpp

#include <cstdlib>
#include "Sort\MergeSort.hpp"
#include "Search\FibSearch.hpp"

#define DEFAULT_CAPACITY  3 //默认的初始容量（实际应用中可设置为更大）

template <typename T>
class Vector { //向量模板类

    protected:
        int _size; int _capacity;  T* _elem; //规模、容量、数据区
        void copyFrom ( T const* A, int lo, int hi ); //复制数组区间A[lo, hi)
        void expand(); //空间不足时扩容
        void shrink(); //装填因子过小时压缩

    public:
        // 构造函数
        Vector ( int c = DEFAULT_CAPACITY, int s = 0, T v = 0 ) //容量为c、规模为s、所有元素初始为v
        { _elem = new T[_capacity = c]; for ( _size = 0; _size < s; _elem[_size++] = v ); } //s<=c
        Vector ( T const* A, int n ) { copyFrom ( A, 0, n ); } //数组整体复制
        Vector ( T const* A, int lo, int hi ) { copyFrom ( A, lo, hi ); } //区间
        Vector ( Vector<T> const& V ) { copyFrom ( V._elem, 0, V._size ); } //向量整体复制
        Vector ( Vector<T> const& V, int lo, int hi ) { copyFrom ( V._elem, lo, hi ); } //区间
        // 析构函数
        ~Vector() { delete [] _elem; } //释放内部空间
        //只读访问接口
        int size() const { return _size; } //规模
        bool empty() const { return !_size; } //判空
        int find ( T const& e ) const { return find ( e, 0, _size ); } //无序向量整体查找
        int find ( T const& e, int lo, int hi ) const; //无序向量区间查找
        int search ( T const& e ) const //有序向量整体查找
        { return ( 0 >= _size ) ? -1 : search ( e, 0, _size ); }
        int search ( T const& e, int lo, int hi ) const; //有序向量区间查找
        //可写访问接口
        T& operator[] ( int r ){ return _elem[r]; } //重载下标操作符，可以类似于数组形式引用各元素
        Vector<T> & operator= ( Vector<T> const& ); //重载赋值操作符，以便直接克隆向量
        T remove ( int r ); //删除秩为r的元素
        int remove ( int lo, int hi ); //删除秩在区间[lo, hi)之内的元素
        int insert ( int r, T const& e ); //插入元素
        int insert ( T const& e ) { return insert ( _size, e ); } //默认作为末元素插入
        void sort ( int lo, int hi ); //对[lo, hi)排序
        void sort() { sort ( 0, _size ); } //整体排序
        void unsort ( int lo, int hi ); //对[lo, hi)置乱
        void unsort() { unsort ( 0, _size ); } //整体置乱
        int deduplicate(); //无序去重
        int uniquify(); //有序去重
        //遍历
        void traverse ( void (* ) ( T& ) ); //遍历（使用函数指针，只读或局部性修改）
        template <typename VST> void traverse ( VST& ); //遍历（使用函数对象，可全局性修改）
};

template <typename T>
void Vector<T>::copyFrom( T const* A, int lo, int hi ){ //以数组区间A[lo, hi)为蓝本复制向量
    _elem = new T[ _capacity = 2*(hi-lo) ]; //分配空间
    _size = 0; //规模清零
    while(lo<hi){ //A[lo, hi)内的元素逐一
        _elem[_size++] = A[lo++]; //复制至_elem[0, hi - lo)
    }
}

template <typename T>
void Vector<T>::expand(){ //向量空间不足时扩容
    if(_size < _capacity)return; //尚未满员时，不必扩容
    if(_capacity < DEFAULT_CAPACITY)_capacity = DEFAULT_CAPACITY; //不低于最小容量
    T* oldElem = _elem; //复制原向量内容
    _elem = new T[_capacity <<= 1]; //容量加倍
    for(int i = 0;i < _size;i++)_elem[i] = oldElem[i]; //复制原向量内容（T为基本类型，或已重载赋值操作符'='）
    delete [] oldElem; //释放原空间
}

template <typename T>
void Vector<T>::shrink(){ //装填因子过小时压缩向量所占空间
    if(_capacity < DEFAULT_CAPACITY << 1)return; //不致收缩到DEFAULT_CAPACITY以下
    if(_size << 2 > _capacity)return; //以25%为界
    T* oldElem = _elem; //复制原向量内容
    _elem = new T[_capacity >>= 1]; //容量减半
    for(int i = 0;i < _size;i++)_elem[i] = oldElem[i]; //复制原向量内容（T为基本类型，或已重载赋值操作符'='）
    delete [] oldElem; //释放原空间
}

template <typename T> //无序向量的顺序查找：返回最后一个元素e的位置；失败时，返回lo - 1
int Vector<T>::find ( T const& e, int lo, int hi ) const { //assert: 0 <= lo < hi <= _size
    while ( ( lo < hi-- ) && ( e != _elem[hi] ) ); //从后向前，顺序查找
    return hi; //若hi < lo，则意味着失败；否则hi即命中元素的秩
}

template <typename T> //在有序向量的区间[lo, hi)内，确定不大于e的最后一个节点的秩
int Vector<T>::search ( T const& e, int lo, int hi ) const { //assert: 0 <= lo < hi <= _size
    return fibSearch ( _elem, e, lo, hi );
}

template <typename T>
Vector<T>& Vector<T>::operator= ( Vector<T> const& V ) { //深复制
    if ( _elem ) delete [] _elem; //释放原有内容
    copyFrom ( V._elem, 0, V.size() ); //整体复制
    return *this; //返回当前对象的引用，以便链式赋值
}

template <typename T>
T Vector<T>::remove ( int r ) { //删除向量中秩为r的元素，0 <= r < size
    T e = _elem[r]; //备份被删除元素
    remove ( r, r + 1 ); //调用区间删除算法，等效于对区间[r, r + 1)的删除
    return e; //返回被删除元素
}

template <typename T>
int Vector<T>::remove ( int lo, int hi ) { //删除区间[lo, hi)
    if ( lo == hi ) return 0; //出于效率考虑，单独处理退化情况，比如remove(0, 0)
    while ( hi < _size ) _elem[lo++] = _elem[hi++]; //[hi, _size)顺次前移hi - lo个单元
    _size = lo; //更新规模，直接丢弃尾部[lo, _size = hi)区间
    shrink(); //若有必要，则缩容
    return hi - lo; //返回被删除元素的数目
}

template <typename T> //将e作为秩为r元素插入
int Vector<T>::insert ( int r, T const& e ) { //assert: 0 <= r <= size
    expand(); //若有必要，扩容
    for ( int i = _size; i > r; i-- ) _elem[i] = _elem[i-1]; //自后向前，后继元素顺次后移一个单元
    _elem[r] = e;//置入新元素
    _size++; //更新容量
    return r; //返回秩
}

template <typename T>
void Vector<T>::sort ( int lo, int hi ) { //向量区间[lo, hi)排序
    mergeSort( _elem, lo, hi );
}

template <typename T>
void Vector<T>::unsort ( int lo, int hi ) { //等概率随机置乱区间[lo, hi)
    T* V = _elem + lo; //将子向量_elem[lo, hi)视作另一向量V[0, hi - lo)
    for ( int i = hi - lo; i > 0; i-- ) {//自后向前
        int r = rand() % i;
        T tmp = V[i - 1];V[i - 1] = V[r];V[r] = tmp; //将V[i - 1]与V[0, i)中某一元素随机交换
    }
}

template <typename T>
int Vector<T>::deduplicate() { //删除无序向量中重复元素（高效版）
    int oldSize = _size; //记录原规模
    int i = 1; //从_elem[1]开始
    while ( i < _size ) //自前向后逐一考查各元素_elem[i]
       ( find ( _elem[i], 0, i ) < 0 ) ? //在其前缀中寻找与之雷同者（至多一个）
       i++ : remove ( i ); //若无雷同则继续考查其后继，否则删除雷同者
    return oldSize - _size; //向量规模变化量，即被删除元素总数
}

template <typename T>
int Vector<T>::uniquify() { //有序向量重复元素剔除算法（高效版）
    int i = 0, j = 0; //各对互异“相邻”元素的秩
    while ( ++j < _size ) //逐一扫描，直至末元素
        if ( _elem[i] != _elem[j] ) //跳过雷同者
            _elem[++i] = _elem[j]; //发现不同元素时，向前移至紧邻于前者右侧
    _size = ++i;
    shrink(); //直接截除尾部多余元素
    return j - i; //向量规模变化量，即被删除元素总数
}

template <typename T>
void Vector<T>::traverse ( void ( *visit ) ( T& ) ){ //借助函数指针机制
    for ( int i = 0; i < _size; i++ ) visit ( _elem[i] ); //遍历向量
}
 
template <typename T> template <typename VST> //元素类型、操作器
void Vector<T>::traverse ( VST& visit ){ //借助函数对象机制
    for ( int i = 0; i < _size; i++ ) visit ( _elem[i] ); //遍历向量
}

#endif
