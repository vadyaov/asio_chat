#include <iostream>
#include <functional>
#include <asio.hpp>

class printer {
  public:
    printer(asio::io_context& io)
      : timer_(io, asio::chrono::seconds(1)),
      count_{0}
    {
      timer_.async_wait(std::bind(&printer::print, this));
    }

    ~printer() {
      std::cout << "Final count is " << count_ << std::endl;
    }

    void print() {
      if (count_ < 5) {
        std::cout << count_ << std::endl;
        count_++;
        timer_.expires_at(timer_.expiry() + asio::chrono::seconds(1));
        timer_.async_wait(std::bind(&printer::print, this));
      }
    }

  private:
    asio::steady_timer timer_;
    int count_;
};

int main() {
  asio::io_context io_context;

  printer p(io_context);
  io_context.run();

  return 0;
}