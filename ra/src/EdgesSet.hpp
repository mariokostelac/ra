
#ifndef _EDGES_SET
#define _EDGES_SET

#include <map>
#include <vector>
#include <utility>

namespace Graph {

  template<typename T>
    using Map = std::map<int, std::vector<std::pair<int, T>>>;

  template <typename T>
    class EdgesSet {
      public:
      void add(int src, int dst, T edge_info);
      std::vector<std::pair<int, T>>& operator[](int src);

      uint32_t size();
      typename Map<T>::iterator begin();
      typename Map<T>::iterator end();

      private:
        Map<T> edges_;
    };

  template <typename T>
    std::vector<std::pair<int, T>>& EdgesSet<T>::operator[](int src) {
      return edges_[src];
    }

  template <typename T>
    void EdgesSet<T>::add(int src, int dst, T edge_info) {
      edges_[src].emplace_back(dst, edge_info);
    }

  template <typename T>
    uint32_t EdgesSet<T>::size() {
      return edges_.size();
    }

  template <typename T>
    typename Map<T>::iterator EdgesSet<T>::begin() {
      return edges_.begin();
    }

  template <typename T>
    typename Map<T>::iterator EdgesSet<T>::end() {
      return edges_.end();
    }
}

#endif
