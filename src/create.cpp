#include <dogitoys/create.hpp>

using ArgsValueType = AGizmo::Args::ValueType;

int main(int argc, char *argv[]) {
  AGizmo::Args::Arguments args{};
  args.addArgument("first", "first argument", ArgsValueType::Integer);
  args.addArgument(2, "fifth", "fifth argument", ArgsValueType::Integer);
  args.addArgument("second", "first argument", ArgsValueType::String, 's');
  args.addArgument(1, "fourth", "first argument", ArgsValueType::String);
  args.addArgument("third", "first argument", ArgsValueType::Bool, 't');

  args.parse(argc, argv);
}
