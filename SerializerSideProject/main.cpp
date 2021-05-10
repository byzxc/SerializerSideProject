
#include "Serialization.hpp"

int main()
{
    JSON::Serialization serializer;
    serializer.DeserializeFromFile("json\\testing.json");
    //serializer.SerializeToFile("json\\testing.json");
}