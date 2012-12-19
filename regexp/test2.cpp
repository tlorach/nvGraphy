#include <regexp/Pattern.h>
#include <regexp/Matcher.h>
#include <cstdio>

int main()
{
  Pattern * p = Pattern::compile("(?s)(?i)(?:^|.*[^a-zA-Z])BLUE(?:[^a-zA-Z].*|$)");
  Matcher * m0 = p->createMatcher("BLUE");
  Matcher * m1 = p->createMatcher("x BLUE");
  Matcher * m2 = p->createMatcher("BLUE x");
  Matcher * m3 = p->createMatcher("x BLUE x");

  printf("m0->matches(): %s\n\n\n", m0->matches() ? "true" : "false");
  printf("m1->matches(): %s\n\n\n", m1->matches() ? "true" : "false");
  printf("m2->matches(): %s\n\n\n", m2->matches() ? "true" : "false");
  printf("m3->matches(): %s\n\n\n", m3->matches() ? "true" : "false");

  p->print();

  return 0;
}
