#include <deque>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <cstring>
using namespace std;

enum class Stype:int {dot=1,hbar,vbar,square};

template <class _Tp>  
struct my_equal_to : public binary_function<_Tp, _Tp, bool>  
{  
    bool operator()(const _Tp& __x, const _Tp& __y) const  
    { return strcmp( __x, __y ) == 0; }  
};

struct Hash_Func{
    //BKDR hash algorithm
    /*
     *size_t operator()(char * str) const
     *{
     *    size_t seed = 131;//31  131 1313 13131131313 etc//
     *    size_t hash = 0;
     *    while(*str)
     *    {
     *        hash = (hash * seed) + (*str);
     *        str++;
     *    }
     *    return hash & (0x7FFFFFFF);
     *}
     */

    size_t operator()(char* str) const
    {
        size_t hash = 5381;
        int c;
        while (c=*str++)
        {
          hash = ((hash<<5)+hash) + c;
          //hash = hash*33^c;
        }
        return hash & (0x7FFFFFFF);
    }
};

typedef unordered_set<char*,Hash_Func,my_equal_to<char*>> my_unordered_set;

class Shape
{
public:
  Shape (Stype _t, int _p, int _l):type(_t),top(_p),left(_l){};
  Shape (){};
  Shape (Stype _t):type(_t){};
  //virtual ~Shape ();
  static const int bottom_offset[4];
  static const int right_offset[4];

  inline int bottom() const {
    return top+bottom_offset[static_cast<int>(type)-1];
  };

  inline int right() const {
    return left+right_offset[static_cast<int>(type)-1];
  };

  int top=1;
  int left=1;
  Stype type=Stype::dot;

};

const int Shape::bottom_offset[4] = { 0, 0, 1, 1 };
const int Shape::right_offset[4] = { 0, 1, 0, 1 };

class State
{
public:
  State (const char * _str, int _step=0)
  {
    parsemark(_str);
    step = _step;
  };

  static const int rows = 5;
  static const int cols = 4;
  static const int bsize= 20;
  static int squareindex;

  std::vector<Shape> _shapes;
  State* parent;
  int step = 0;

  inline bool empty(const char* mask,int x, int y){
    return !(x<0||x>rows-1||y<0||y>cols-1) && mask[x*cols+y] == '0';
    /*
     *if(x<0||x>rows-1||y<0||y>cols-1){
     *  return false;
     *}else
     *{
     *  return mask[x*cols+y] == '0';
     *}
     */
  };

  inline bool issolved(){
    return (_shapes[squareindex].top==3 && _shapes[squareindex].left == 1); 
  };

  inline void setvalue(char* mask,char value,int x, int y)const{
    mask[x*cols+y] = value;
  };

  char* tomask(){
    char* mask = new char[bsize];
    memset(mask,'0',bsize*sizeof(char));
    for(const auto& s:_shapes){
      int x = s.top;
      int y = s.left;
      switch(s.type)
      {
        case Stype::dot:
          setvalue(mask,'D',x,y);
          break;
        case Stype::hbar:
          setvalue(mask,'H',x,y);
          setvalue(mask,'H',x,y+1);
          break;
        case Stype::vbar:
          setvalue(mask,'V',x,y);
          setvalue(mask,'V',x+1,y);
          break;
        case Stype::square:
          setvalue(mask,'S',x,y);
          setvalue(mask,'S',x,y+1);
          setvalue(mask,'S',x+1,y);
          setvalue(mask,'S',x+1,y+1);
          break;
		default:;
      }
    }
    return mask;
  };

  template<typename FUNC>
  void nextmoves(const FUNC& func)
  {
    //static_assert(std::is_convertible<FUNC, std::function<void(const State&)>>::value,
                  //"func must be callable with a 'const State&' parameter.");
	const char* mask = tomask();
    for(unsigned int i=0; i<_shapes.size();i++){
      // try to move up
      auto & s = _shapes[i];
	  if (s.top>0 && empty(mask, s.top - 1, s.left) && empty(mask,s.top - 1, s.right()))
      {
        State next = *this;
        next.parent = this;
        next.step++;
        next._shapes[i].top--;
        func(next);
      }
      // move down
	  if (s.bottom()<rows - 1 && empty(mask, s.bottom() + 1, s.left) && empty(mask,s.bottom() + 1, s.right()))
      {
        State next = *this;
        next.parent = this;
        next.step++;
        next._shapes[i].top++;
        func(next);
      }
      // move left
	  if (s.left>0 && empty(mask, s.top, s.left - 1) && empty(mask, s.bottom(), s.left - 1))
      {
        State next = *this;
        next.parent = this;
        next.step++;
        next._shapes[i].left--;
        func(next);
      }
      // move right
	  if (s.right()<cols - 1 && empty(mask, s.top, s.right() + 1) && empty(mask,s.bottom(), s.right() + 1))
      {
        State next = *this;
        next.parent = this;
        next.step++;
        next._shapes[i].left++;
        func(next);
      }
    }

  };
  
  bool parsemark(const char* _str){
    int status[bsize] = {0};
    for(int i=0;i<bsize;i++){
      if(status[i]==0 && _str[i]!='0'){
        status[i] = 1;
        switch(_str[i])
        {
          case 'D':
          case 'd':
            _shapes.push_back(Shape(Stype::dot,i/cols,i%cols));
            break;
          case 'H':
          case 'h':
            _shapes.push_back(Shape(Stype::hbar,i/cols,i%cols));
            status[i+1]=1;
            break;
          case 'V':
          case 'v':
            _shapes.push_back(Shape(Stype::vbar,i/cols,i%cols));
            status[i+cols]=1;
            break;
          case 's':
          case 'S':
            _shapes.push_back(Shape(Stype::square,i/cols,i%cols));
            squareindex = _shapes.size()-1;            
            status[i+1]=1;
            status[i+cols]=1;
            status[i+cols+1]=1;
            break;
          default:
            return false; 
        }
      }
    }
    return true;
  };

};

int State::squareindex=-1;

int main(int argc, char *argv[])
{
  my_unordered_set seen;
  std::deque<State> steps;
  //std::stack<State*> solution;

  /*
   *string str = "VSSVVSSVVHHVVDDVD00D";
   *string str = "VV0VVV0VDSSDDSSDHHHH";
   *const char* gamestr = str.c_str();
   */

  char *gamestr = new char[20];
  cout<<"Please input the initial game"<<endl;
  cin>>gamestr;
  cout<<"Initial Game:"<<gamestr<<endl;
  cout<<"Press Enter to continue"<<endl;
  cin.clear();
  cin.get();

  auto game = State(gamestr);
  steps.push_back(game);
  seen.insert(game.tomask());

  while(!steps.empty()){
    State curr = steps.front();
    steps.pop_front();

    //cout<<"SquareIndex:"<<State::squareindex<<endl;
    if(curr.issolved()){
      std::cout<<"Optimal steps:"<<curr.step<<std::endl;
      /*
       *State* currptr = &curr;
       *while(currptr!= NULL){
       *  solution.push(currptr);
       *  currptr = currptr->parent;
       *}
       */
      return 1;
    }

    curr.nextmoves([&seen, &steps](State& next) {
      auto result = seen.insert(next.tomask());
      if (result.second)
        steps.push_back(next);
    });
  }
  std::cout << "not found solution" << std::endl;
  return 0;
}
