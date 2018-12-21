#include <dogitoys/create.hpp>

int main(int argc, char *argv[]) {
  AGizmo::Args::Arguments args{};
  args.addArgument(1, "first", "first argument");

  args.parse(argc, argv);
}
