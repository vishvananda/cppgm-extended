using Throwing = int (*)();
using Safe = int (*)() noexcept;
Safe safe; Throwing ordinary;
