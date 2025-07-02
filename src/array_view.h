#ifndef ARRAY_VIEW_H
#define ARRAY_VIEW_H

template<typename T>
struct Array_View {
    T*    data;
    usize len;
};

template<typename T>
Array_View<T> array_view_from(std::vector<T> vec);

template<typename T>
Array_View<T> array_view_from(T* data[], usize len);

#endif // ARRAY_VIEW_H
