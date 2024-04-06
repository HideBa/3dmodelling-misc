
#include "../io.cpp"
#include "../types.h"
#include <cassert>
#include <map>

void test_read_obj() {
  const string test_file = "../src/tests/testdata/open_house_ifc4.obj";
  {
    ifstream input_stream;
    input_stream.open(test_file);
    assert(true == true);
    auto result = read_obj(input_stream);
    assert(result.first.size() ==
           35); // manually counted the number of group in the testing
    assert(!result.second.empty());
    for (auto const &bim_obj_pair : result.first) {
      string name = bim_obj_pair.first;
      BIMObject bim_obj = bim_obj_pair.second;
      assert(!bim_obj.shells.empty());
      assert((!name.empty()));
    }
    assert(result.first["product-ea106fba-c8b5-46db-8aa1-a2217e8191b1-body"]
               .shells.size() == 48);
  }
}

int main() {
  test_read_obj();
  return 0;
}