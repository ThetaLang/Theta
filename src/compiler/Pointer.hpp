namespace Theta {
  enum PointerType {
    Function,
    Closure,
    Data
  };
  
  template<PointerType type>
  class Pointer {
  public:  
    Pointer() : address(-1) {}
    Pointer(int addr) : address(addr) {}

    PointerType getType() { return type; }
    
    int getAddress() { return address; }

  private:
    int address;
  };
}
