/**
  From the author (Jeff Stuart)
  "
  Let me start by saying this file is pretty big. If you feel up to it, you can
  try making changes yourself, but you would be better off to just email me at
  stuart@cs.ucdavis.edu if you think there is a bug, or have something useful you
  would like added. This project is very "near and dear" to me, so I am fairly
  quick to make bug fixes. The header files for Pattern and Matcher are fairly
  well documented and the function names are pretty self-explanatory, but if you
  are having any trouble, feel free to email me at stuart@cs.ucdavis.edu.

  If you email me, make sure you put something like C++RE in the subject because
  I tend to delete email if I don't recognize the name and the subject is
  something like "I Need Your Help" or "Got A Second" or "I Found It".
  "
 */

/*
  Detailed documentation is provided in this class' header file

  @author   Jeffery Stuart
  @since    November 24th, 2007
  @version  1.08.00
*/

#ifdef _WIN32
  #pragma warning(push)
  #pragma warning(disable:4996)
  #define str_icmp stricmp
#else
  #define str_icmp strcasecmp
#endif

#include <regexp/Pattern.h>
#include <regexp/Matcher.h>
#include <cstdio>
#include <cstring>
#include <typeinfo>
#include <algorithm>

std::map<std::string, Pattern *> Pattern::compiledPatterns;
std::map<std::string, std::pair<std::string, unsigned long> > Pattern::registeredPatterns;

const int Pattern::MIN_QMATCH = 0x00000000;
const int Pattern::MAX_QMATCH = 0x7FFFFFFF;

const unsigned long Pattern::CASE_INSENSITIVE       = 0x01;
const unsigned long Pattern::LITERAL                = 0x02;
const unsigned long Pattern::DOT_MATCHES_ALL        = 0x04;
const unsigned long Pattern::MULTILINE_MATCHING     = 0x08;
const unsigned long Pattern::UNIX_LINE_MODE         = 0x10;

Pattern::Pattern(const std::string & rhs)
{
  matcher = NULL;
  pattern = rhs;
  curInd = 0;
  groupCount = 0;
  nonCapGroupCount = 0;
  error = 0;
  head = NULL;
}
// convenience function in case we want to add any extra debugging output
void Pattern::raiseError()
{
  switch (pattern[curInd - 1])
  {
  case '*':
  case ')':
  case '+':
  case '?':
  case ']':
  case '}':
    fprintf(stderr, "%s\n%*c^\n", pattern.c_str(), curInd - 1, ' ');
    fprintf(stderr, "Syntax Error near here. Possible unescaped meta character.\n");
    break;
  default:
    fprintf(stderr, "%s\n%*c^\n", pattern.c_str(), curInd - 1, ' ');
    fprintf(stderr, "Syntax Error near here. \n");
    break;
  }
  error = 1;
}
NFANode * Pattern::registerNode(NFANode * node)
{
  nodes[node] = 1;
  return node;
}

std::string Pattern::classUnion      (std::string s1, std::string s2)  const
{
  char out[300];
  std::sort(s1.begin(), s1.end());
  std::sort(s2.begin(), s2.end());
  *std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), out) = 0;
  return out;
}
std::string Pattern::classIntersect  (std::string s1, std::string s2)  const
{
  char out[300];
  std::sort(s1.begin(), s1.end());
  std::sort(s2.begin(), s2.end());
  *std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), out) = 0;
  return out;
}
std::string Pattern::classNegate     (std::string s1)                  const
{
  char out[300];
  int i, ind = 0;
  std::map<char, bool> m;

  for (i = 0; i < (int)s1.size(); ++i) m[s1[i]] = 1;
  for (i = 0xFF; i >= 0; --i) if (m.find((char)i) == m.end()) out[ind++] = (char)i;
  out[ind] = 0;
  return std::string(out, ind);
}
std::string Pattern::classCreateRange(char low, char hi)    const
{
  char out[300];
  int ind = 0;
  while (low != hi) out[ind++] = low++;
  out[ind++] = low;
  return std::string(out, ind);
}

int Pattern::getInt(int start, int end)
{
  int ret = 0;
  for (; start <= end; ++start) ret = ret * 10 + (pattern[start] - '0');
  return ret;
}
bool Pattern::quantifyCurly(int & sNum, int & eNum)
{
  bool good = 1;
  int i, ci = curInd + 1;
  int commaInd = ci, endInd = ci, len = pattern.size();
  sNum = eNum = 0;

  while (endInd   < len     && pattern[endInd  ] != '}') ++endInd;
  while (commaInd < endInd  && pattern[commaInd] != ',') ++commaInd;
  if (endInd >= len) { raiseError(); return 0; }
  for (i = ci; good && i < endInd; ++i) if (i != commaInd && !isdigit(pattern[i])) good = 0;
  if (!good && commaInd < endInd) { raiseError(); return 0; }
  if (!good) return 0;
  /* so now everything in here is either a comma (and there is at most one comma) or a digit */
  if (commaInd == ci) // {,*}
  {
    if (endInd == commaInd + 1)    { sNum = MIN_QMATCH;               eNum = MAX_QMATCH;                        } // {,} = *
    else                           { sNum = MIN_QMATCH;               eNum = getInt(commaInd + 1, endInd - 1);  } // {,+}
  }
  else if (commaInd == endInd - 1) { sNum = getInt(ci, commaInd - 1); eNum = MAX_QMATCH;                        } // {+,}
  else if (commaInd == endInd)     { sNum = getInt(ci, endInd - 1);   eNum = sNum;                              } // {+}
  else                             { sNum = getInt(ci, commaInd - 1); eNum = getInt(commaInd + 1, endInd - 1);  } // {+,+}
  curInd = endInd + 1;
  return 1;
}
NFANode * Pattern::quantifyGroup(NFANode * start, NFANode * stop, const int gn)
{
  NFANode * newNode = NULL;
  int type = 0;

  if (curInd < (int)pattern.size())
  {
    char ch = (curInd + 1 >= (int)pattern.size()) ? -1 : pattern[curInd + 1];
    switch (pattern[curInd])
    {
    case '*':
      ++curInd;
      switch (ch)
      {
      case '?': ++curInd; type = 1; break;
      case '+': ++curInd; type = 2; break;
      }
      newNode = registerNode(new NFAGroupLoopPrologueNode(gn));
      newNode->next = registerNode(new NFAGroupLoopNode(start, MIN_QMATCH, MAX_QMATCH, gn, type));
      stop->next = newNode->next;
      return newNode;
    case '?':
      ++curInd;
      switch (ch)
      {
      case '?': ++curInd; type = 1; break;
      case '+': ++curInd; type = 2; break;
      }
      newNode = registerNode(new NFAGroupLoopPrologueNode(gn));
      newNode->next = registerNode(new NFAGroupLoopNode(start, MIN_QMATCH, 1, gn, type));
      stop->next = newNode->next;
      return newNode;
    case '+':
      ++curInd;
      switch (ch)
      {
      case '?': ++curInd; type = 1; break;
      case '+': ++curInd; type = 2; break;
      }
      newNode = registerNode(new NFAGroupLoopPrologueNode(gn));
      newNode->next = registerNode(new NFAGroupLoopNode(start, 1, MAX_QMATCH, gn, type));
      stop->next = newNode->next;
      return newNode;
    case '{':
      {
        int s, e;
        if (quantifyCurly(s, e))
        {
          ch = (curInd < (int)pattern.size()) ? pattern[curInd] : -1;
          switch (ch)
          {
          case '?': ++curInd; type = 1; break;
          case '+': ++curInd; type = 2; break;
          }
          newNode = registerNode(new NFAGroupLoopPrologueNode(gn));
          newNode->next = registerNode(new NFAGroupLoopNode(start, s, e, gn, type));
          stop->next = newNode->next;
          return newNode;
        }
      }
    default:
      break;
    }
  }
  return NULL;
}

NFANode * Pattern::quantify(NFANode * newNode)
{
  if (curInd < (int)pattern.size())
  {
    char ch = (curInd + 1 >= (int)pattern.size()) ? -1 : pattern[curInd + 1];
    switch (pattern[curInd])
    {
    case '*':
      ++curInd;
      switch (ch)
      {
      case '?': ++curInd; newNode = registerNode(new NFALazyQuantifierNode      (this, newNode, MIN_QMATCH, MAX_QMATCH)); break;
      case '+': ++curInd; newNode = registerNode(new NFAPossessiveQuantifierNode(this, newNode, MIN_QMATCH, MAX_QMATCH)); break;
      default:            newNode = registerNode(new NFAGreedyQuantifierNode    (this, newNode, MIN_QMATCH, MAX_QMATCH)); break;
      }
      break;
    case '?':
      ++curInd;
      switch (ch)
      {
      case '?': ++curInd; newNode = registerNode(new NFALazyQuantifierNode      (this, newNode, MIN_QMATCH, 1)); break;
      case '+': ++curInd; newNode = registerNode(new NFAPossessiveQuantifierNode(this, newNode, MIN_QMATCH, 1)); break;
      default:            newNode = registerNode(new NFAGreedyQuantifierNode    (this, newNode, MIN_QMATCH, 1)); break;
      }
      break;
    case '+':
      ++curInd;
      switch (ch)
      {
      case '?': ++curInd; newNode = registerNode(new NFALazyQuantifierNode      (this, newNode, 1, MAX_QMATCH)); break;
      case '+': ++curInd; newNode = registerNode(new NFAPossessiveQuantifierNode(this, newNode, 1, MAX_QMATCH)); break;
      default:            newNode = registerNode(new NFAGreedyQuantifierNode    (this, newNode, 1, MAX_QMATCH)); break;
      }
      break;
    case '{':
      {
        int s, e;
        if (quantifyCurly(s, e))
        {
          ch = (curInd < (int)pattern.size()) ? pattern[curInd] : -1;
          switch (ch)
          {
          case '?': ++curInd; newNode = registerNode(new NFALazyQuantifierNode      (this, newNode, s, e)); break;
          case '+': ++curInd; newNode = registerNode(new NFAPossessiveQuantifierNode(this, newNode, s, e)); break;
          default:            newNode = registerNode(new NFAGreedyQuantifierNode    (this, newNode, s, e)); break;
          }
        }
      }
      break;
    default:
      break;
    }
  }
  return newNode;
}
std::string Pattern::parseClass()
{
  std::string t, ret = "";
  char ch, c1, c2;
  bool inv = 0, neg = 0, quo = 0;

  if (curInd < (int)pattern.size() && pattern[curInd] == '^')
  {
    ++curInd;
    neg = 1;
  }
  while (curInd < (int)pattern.size() && pattern[curInd] != ']')
  {
    ch = pattern[curInd++];
    if (ch == '[')
    {
      t = parseClass();
      ret = classUnion(ret, t);
    }
    /*else if (ch == '-')
    {
      raiseError();
      curInd = pattern.size();
    }*/
    else if (ch == '&' && curInd < (int)pattern.size() && pattern[curInd] == '&')
    {
      if (pattern[++curInd] != '[')
      {
        raiseError();
        curInd = pattern.size();
      }
      else
      {
        ++curInd;
        t = parseClass();
        ret = classIntersect(ret, t);
      }
    }
    else if (ch == '\\')
    {
      t = parseEscape(inv, quo);
      if (quo)
      {
        raiseError();
        curInd = pattern.size();
      }
      else if (inv || t.size() > 1) // cant be part of a range (a-z)
      {
        if (inv) t = classNegate(t);
        ret = classUnion(ret, t);
      }
      else if (curInd < (int)pattern.size() && pattern[curInd] == '-') // part of a range (a-z)
      {
        c1 = t[0];
        ++curInd;
        if (curInd >= (int)pattern.size()) raiseError();
        else
        {
          c2 = pattern[curInd++];
          if (c2 == '\\')
          {
            t = parseEscape(inv, quo);
            if (quo)
            {
              raiseError();
              curInd = pattern.size();
            }
            else if (inv || t.size() > 1) raiseError();
            else ret = classUnion(ret, classCreateRange(c1, c2));
          }
          else if (c2 == '[' || c2 == ']' || c2 == '-' || c2 == '&')
          {
            raiseError();
            curInd = pattern.size();
          }
          else ret = classUnion(ret, classCreateRange(c1, c2));
        }
      }
      else
      {
        ret = classUnion(ret, t);
      }
    }
    else if (curInd < (int)pattern.size() && pattern[curInd] == '-')
    {
      c1 = ch;
      ++curInd;
      if (curInd >= (int)pattern.size()) raiseError();
      else
      {
        c2 = pattern[curInd++];
        if (c2 == '\\')
        {
          t = parseEscape(inv, quo);
          if (quo)
          {
            raiseError();
            curInd = pattern.size();
          }
          else if (inv || t.size() > 1) raiseError();
          else ret = classUnion(ret, classCreateRange(c1, c2));
        }
        else if (c2 == '[' || c2 == ']' || c2 == '-' || c2 == '&')
        {
          raiseError();
          curInd = pattern.size();
        }
        else
        {
          ret = classUnion(ret, classCreateRange(c1, c2));
        }
      }
    }
    else
    {
      ret += " ";
      ret[ret.size() - 1] = ch;
    }
  }
  if (curInd >= (int)pattern.size() || pattern[curInd] != ']')
  {
    raiseError();
    ret = "";
  }
  else
  {
    ++curInd;
    if (neg) ret = classNegate(ret);
  }
  return ret;
}
std::string Pattern::parsePosix()
{
  std::string s7 = pattern.substr(curInd, 7);
  if (s7 == "{Lower}")  { curInd += 7; return "abcdefghijklmnopqrstuvwxyz";                                                                       }
  if (s7 == "{Upper}")  { curInd += 7; return "ABCDEFGHIJKLMNOPQRSTUVWXYZ";                                                                       }
  if (s7 == "{Alpha}")  { curInd += 7; return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";                                             }
  if (s7 == "{Digit}")  { curInd += 7; return "0123456789";                                                                                       }
  if (s7 == "{Alnum}")  { curInd += 7; return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";                                   }
  if (s7 == "{Punct}")  { curInd += 7; return "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";                                                               }
  if (s7 == "{Graph}")  { curInd += 7; return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"; }
  if (s7 == "{Print}")  { curInd += 7; return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"; }
  if (s7 == "{Blank}")  { curInd += 7; return " \t";                                                                                              }
  if (s7 == "{Space}")  { curInd += 7; return " \t\n\x0B\f\r";                                                                                    }
  if (s7 == "{Cntrl}")
  {
    int i;
    std::string s = " ";

    for (i = 0; i < 5; ++i) s += s;
    s += " ";
    for (i = 0; i <= 0x1F; ++i) s[i] = i;
    s[0x20] = 0x7F;
    curInd += 7;
    return s;
  }
  if (s7 == "{ASCII}")
  {
    std::string s(0x80, ' ');
    for (int i = 0; i < 0x80; ++i) s[i] = i;
    curInd += 7;
    return s;
  }
  if (pattern.substr(curInd, 8) == "{XDigit}") { curInd += 8; return "abcdefABCDEF0123456789"; }
  raiseError();
  return "";
}
NFANode * Pattern::parseBackref()
{
  #define is_dig(x) ((x) >= '0' && (x) <= '9')
  #define to_int(x) ((x) - '0')
  int ci = curInd;
  int oldRef = 0, ref = 0;

  while (ci < (int)pattern.size() && is_dig(pattern[ci]) && (ref < 10 || ref < groupCount))
  {
    oldRef = ref;
    ref = ref * 10 + to_int(pattern[ci++]);
  }
  if (ci == (int)pattern.size())
  {
    oldRef = ref;
    ++ci;
  }
  if (oldRef < 0 || ci <= curInd)
  {
    raiseError();
    return registerNode(new NFAReferenceNode(-1));
  }
  curInd = ci;
  return registerNode(new NFAReferenceNode(ref));

  #undef is_dig
  #undef to_int
}
std::string Pattern::parseOctal()
{
  #define islowoc(x)  ((x) >= '0' && (x) <= '3')
  #define isoc(x)     ((x) >= '0' && (x) <= '7')
  #define fromoc(x)   ((x) - '0')
  int ci = curInd;
  char ch1 = (ci + 0 < (int)pattern.size()) ? pattern[ci + 0] : -1;
  char ch2 = (ci + 1 < (int)pattern.size()) ? pattern[ci + 1] : -1;
  char ch3 = (ci + 2 < (int)pattern.size()) ? pattern[ci + 2] : -1;
  std::string s = " ";

  if (islowoc(ch1) && isoc(ch2))
  {
    curInd += 2;
    s[0] = fromoc(ch1) * 8 + fromoc(ch2);
    if (isoc(ch3))
    {
      ++curInd;
      s[0] = s[0] * 8 + fromoc(ch3);
    }
  }
  else if (isoc(ch1) && isoc(ch2))
  {
    curInd += 2;
    s[0] = fromoc(ch1) * 8 + fromoc(ch2);
  }
  else raiseError();

  return s;
  #undef islowoc
  #undef isoc
  #undef fromoc
}
std::string Pattern::parseHex()
{
  #define to_low(x)   (((x) >= 'A' && (x) <= 'Z') ? ((x) - 'A' + 'a') : (x))
  #define is_dig(x)   ((x) >= '0' && (x) <= '9')
  #define is_hex(x)   (is_dig(x) || (to_low(x) >= 'a' && to_low(x) <= 'f'))
  #define to_int(x)   ((is_dig(x)) ? ((x) - '0') : (to_low(x) - 'a' + 10))

  int ci = curInd;
  char ch1 = (ci + 0 < (int)pattern.size()) ? pattern[ci + 0] : -1;
  char ch2 = (ci + 1 < (int)pattern.size()) ? pattern[ci + 1] : -1;
  std::string s = " ";

  if (is_hex(ch1) && is_hex(ch2))
  {
    curInd += 2;
    s[0] = (to_int(ch1) << 4 & 0xF0) | (to_int(ch2) & 0x0F);
  }

  return s;
  #undef to_low
  #undef is_dig
  #undef is_hex
  #undef to_int
}
std::string Pattern::parseEscape(bool & inv, bool & quo)
{
  char ch = pattern[curInd++];
  std::string classes = "";

  if (curInd > (int)pattern.size())
  {
    raiseError();
    return NULL;
  }

  quo = 0;
  inv = 0;
  switch (ch)
  {
  case 'p': classes = parsePosix();                                                         break;
  case 'P': classes = "!!"; classes += parsePosix();                                        break;
  case 'd': classes = "0123456789";                                                         break;
  case 'D': classes = "!!0123456789";                                                       break;
  case 's': classes = " \t\r\n\f";                                                          break;
  case 'S': classes = "!! \t\r\n\f";                                                        break;
  case 'w': classes = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";    break;
  case 'W': classes = "!!abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";  break;
  case '0': classes = parseOctal(); break;
  case 'x': classes = parseHex();   break;

  case 'Q': quo = 1;        break;
  case 't': classes = "\t"; break;
  case 'r': classes = "\r"; break;
  case 'n': classes = "\n"; break;
  case 'f': classes = "\f"; break;
  case 'a': classes = "\a"; break;
  case 'e': classes = "\r"; break;
  default:  classes = " "; classes[0] = ch; break;
  }
  if (classes.substr(0, 2) == "!!")
  {
    classes = classes.substr(2);
    inv = 1;
  }
  return classes;
}
NFANode * Pattern::parseRegisteredPattern(NFANode ** end)
{
  int i, j;
  std::string s;
  NFANode * ret = NULL;
  for (i = curInd; i < (int)pattern.size() && pattern[i] != '}'; ++i) { }
  if (pattern[i] != '}') { raiseError(); return NULL; }
  if (i == curInd + 1)   { raiseError(); return NULL; } // {}
  if (
      !(
        (pattern[curInd] >= 'a' && pattern[curInd] <= 'z') ||
        (pattern[curInd] >= 'A' && pattern[curInd] <= 'Z') ||
        (pattern[curInd] == '_')
       )
     )
  {
    raiseError();
    return NULL;
  }
  for (j = curInd; !error && j < i; ++j)
  {
    if (
        !(
          (pattern[j] >= 'a' && pattern[j] <= 'z') ||
          (pattern[j] >= 'A' && pattern[j] <= 'Z') ||
          (pattern[j] >= '0' && pattern[j] <= '9') ||
          (pattern[j] == '_')
         )
        )
    {
      raiseError();
      return NULL;
    }
  }
  s = pattern.substr(curInd, i - curInd);
  if (registeredPatterns.find(s) == registeredPatterns.end()) raiseError();
  else
  {
    unsigned long oflags = flags;
    std::string op = pattern;
    int ci = i + 1;

    pattern = registeredPatterns[s].first;
    curInd = 0;
    flags = registeredPatterns[s].second;

    --groupCount;
    ret = parse(0, 0, end);

    pattern = op;
    curInd = ci;
    flags = oflags;
  }
  if (error) { *end = ret = NULL; }
  return ret;
}

// look behind should interpret everything as a literal (except \\) since the
// pattern must have a concrete length
NFANode * Pattern::parseBehind(const bool pos, NFANode ** end)
{
  std::string t = "";
  while (curInd < (int)pattern.size() && pattern[curInd] != ')')
  {
    char ch = pattern[curInd++];
    t += " ";
    if (ch == '\\')
    {
      if (curInd + 1 >= (int)pattern.size())
      {
        raiseError();
        return *end = registerNode(new NFACharNode(' '));
      }
      ch = pattern[curInd++];
    }
    t[t.size() - 1] = ch;
  }
  if (curInd >= (int)pattern.size() || pattern[curInd] != ')') raiseError();
  else ++curInd;
  return *end = registerNode(new NFALookBehindNode(t, pos));
}
NFANode * Pattern::parseQuote()
{
  bool done = 0;
  std::string s = "";

  while (!done)
  {
    if      (curInd >= (int)pattern.size())
    {
      raiseError();
      done = 1;
    }
    else if (pattern.substr(curInd, 2) == "\\E")
    {
      curInd += 2;
      done = 1;
    }
    else if (pattern[curInd] == '\\')
    {
      s += " ";
      s[s.size() - 1] = pattern[++curInd];
      ++curInd;
    }
    else
    {
      s += " ";
      s[s.size() - 1] = pattern[curInd++];
    }
  }
  if ((flags & Pattern::CASE_INSENSITIVE) != 0) return registerNode(new NFACIQuoteNode(s));
  return registerNode(new NFAQuoteNode(s));
}
NFANode * Pattern::parse(const bool inParen, const bool inOr, NFANode ** end, const int orGroup, const bool inOrCap)
{
  NFANode * start, * cur, * next = NULL;
  std::string t;
  int grc = groupCount++;
  bool inv, quo;
  bool ahead = false, pos = false, noncap = false, indep = false;
  unsigned long oldFlags = flags;

  if (inParen)
  {
    if (inOr)
    {
      --groupCount;
      noncap = !inOrCap;
      grc = orGroup;
      if (noncap) cur = start = registerNode(new NFAGroupHeadNode(grc));
      else        cur = start = registerNode(new NFASubStartNode);
    }
    else if (pattern[curInd] == '?')
    {
      ++curInd;
      --groupCount;
      if      (pattern[curInd]           == ':')  { noncap = 1; ++curInd;     grc = --nonCapGroupCount; }
      else if (pattern[curInd]           == '=')  { ++curInd;     ahead = 1;  pos = 1;                  }
      else if (pattern[curInd]           == '!')  { ++curInd;     ahead = 1;  pos = 0;                  }
      else if (pattern.substr(curInd, 2) == "<=") { curInd += 2;  return parseBehind(1, end);           }
      else if (pattern.substr(curInd, 2) == "<!") { curInd += 2;  return parseBehind(0, end);           }
      else if (pattern[curInd]           == '>')  { ++curInd;     indep = 1;                            }
      else
      {
        bool negate = false, done = false;
        while (!done)
        {
          if (curInd >= (int)pattern.size())
          {
            raiseError();
            return NULL;
          }
          else if (negate)
          {
            switch (pattern[curInd])
            {
            case 'i': flags &= ~Pattern::CASE_INSENSITIVE;    break;
            case 'd': flags &= ~Pattern::UNIX_LINE_MODE;      break;
            case 'm': flags &= ~Pattern::MULTILINE_MATCHING;  break;
            case 's': flags &= ~Pattern::DOT_MATCHES_ALL;     break;
            case ':': done = true;                            break;
            case ')':
              ++curInd;
              *end = registerNode(new NFALookBehindNode("", true));
              return *end;
            case '-':
            default: raiseError(); return NULL;
            }
          }
          else
          {
            switch (pattern[curInd])
            {
            case 'i': flags |= Pattern::CASE_INSENSITIVE;     break;
            case 'd': flags |= Pattern::UNIX_LINE_MODE;       break;
            case 'm': flags |= Pattern::MULTILINE_MATCHING;   break;
            case 's': flags |= Pattern::DOT_MATCHES_ALL;      break;
            case ':': done = true;                            break;
            case '-': negate = true;                          break;
            case ')':
              ++curInd;
              *end = registerNode(new NFALookBehindNode("", true));
              return *end;
            default:  raiseError(); return NULL;
            }
          }
          ++curInd;
        }
        noncap = 1;
        grc = --nonCapGroupCount;
      }
      if (noncap) cur = start = registerNode(new NFAGroupHeadNode(grc));
      else        cur = start = registerNode(new NFASubStartNode);
    }
    else cur = start = registerNode(new NFAGroupHeadNode(grc));
  }
  else cur = start = registerNode(new NFASubStartNode);
  while (curInd < (int)pattern.size())
  {
    char ch = pattern[curInd++];

    next = NULL;
    if (error) return NULL;
    switch (ch)
    {
    case '^':
      if ((flags & Pattern::MULTILINE_MATCHING) != 0) next = registerNode(new NFAStartOfLineNode);
      else                                            next = registerNode(new NFAStartOfInputNode);
      break;
    case '$':
      if ((flags & Pattern::MULTILINE_MATCHING) != 0) next = registerNode(new NFAEndOfLineNode);
      else                                            next = registerNode(new NFAEndOfInputNode(0));
      break;
    case '|':
      cur->next = registerNode(new NFAAcceptNode);
      cur = start = registerNode(new NFAOrNode(start, parse(inParen, true, NULL, grc, !noncap)));
      break;
    case '\\':
      if      (curInd < (int)pattern.size())
      {
        bool eoi = 0;
        switch (pattern[curInd])
        {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': next = parseBackref(); break;
        case 'A': ++curInd; next = registerNode(new NFAStartOfInputNode);     break;
        case 'B': ++curInd; next = registerNode(new NFAWordBoundaryNode(0));  break;
        case 'b': ++curInd; next = registerNode(new NFAWordBoundaryNode(1));  break;
        case 'G': ++curInd; next = registerNode(new NFAEndOfMatchNode);       break;
        case 'Z': eoi = 1;
        case 'z': ++curInd; next = registerNode(new NFAEndOfInputNode(eoi));  break;
        default:
          t = parseEscape(inv, quo);
          if (!quo)
          {
            if (t.size() > 1 || inv)
            {
              if ((flags & Pattern::CASE_INSENSITIVE) != 0) next = registerNode(new NFACIClassNode(t, inv));
              else                                          next = registerNode(new NFAClassNode(t, inv));
            }
            else
            {
              next = registerNode(new NFACharNode(t[0]));
            }
          }
          else
          {
            next = parseQuote();
          }
        }
      }
      else raiseError();
      break;
    case '[':
      if ((flags & Pattern::CASE_INSENSITIVE) == 0)
      {
        NFAClassNode * clazz = new NFAClassNode();
        std::string s = parseClass();
        for (int i = 0; i < (int)s.size(); ++i) clazz->vals[s[i]] = 1;
        next = registerNode(clazz);
      }
      else
      {
        NFACIClassNode * clazz = new NFACIClassNode();
        std::string s = parseClass();
        for (int i = 0; i < (int)s.size(); ++i) clazz->vals[tolower(s[i])] = 1;
        next = registerNode(clazz);
      }
      break;
    case '.':
      {
        bool useN = 1, useR = 1;
        NFAClassNode * clazz = new NFAClassNode(1);
        if ((flags & Pattern::UNIX_LINE_MODE)  != 0) useR = 0;
        if ((flags & Pattern::DOT_MATCHES_ALL) != 0) useN = useR = 0;
        if (useN) clazz->vals['\n'] = 1;
        if (useR) clazz->vals['\r'] = 1;
        next = registerNode(clazz);
      }
      break;
    case '(':
      {
        NFANode * end, * t1, * t2;
        t1 = parse(true, false, &end);
        if (!t1) raiseError();
        else if (t1->isGroupHeadNode() && (t2 = quantifyGroup(t1, end, grc)) != NULL)
        {
          cur->next = t2;
          cur = t2->next;
        }
        else
        {
          cur->next = t1;
          cur = end;
        }
      }
      break;
    case ')':
      if (!inParen) raiseError();
      else if (inOr)
      {
        --curInd;
        cur = cur->next = registerNode(new NFAAcceptNode);
        flags = oldFlags;
        return start;
      }
      else
      {
        if (ahead)
        {
          cur = cur->next = registerNode(new NFAAcceptNode);
          flags = oldFlags;
          return *end = registerNode(new NFALookAheadNode(start, pos));
        }
        else if (indep)
        {
          cur = cur->next = registerNode(new NFAAcceptNode);
          flags = oldFlags;
          return *end = registerNode(new NFAPossessiveQuantifierNode(this, start, 1, 1));
        }
        else // capping or noncapping, it doesnt matter
        {
          *end = cur = cur->next = registerNode(new NFAGroupTailNode(grc));
          next = quantifyGroup(start, *end, grc);
          if (next)
          {
            start = next;
            *end = next->next;
          }
          flags = oldFlags;
          return start;
        }
      }
      break;
    case '{': // registered pattern
      cur->next = parseRegisteredPattern(&next);
      if (cur->next) cur = next;
      break;
    case '*':
    case '+':
    case '?':
    case '}':
    case ']':
      raiseError();
      break;
    default:
      if ((flags & Pattern::CASE_INSENSITIVE) != 0) next = registerNode(new NFACICharNode(ch));
      else                                          next = registerNode(new NFACharNode(ch));
      break;
    }
    NFAOrNode * orNode = dynamic_cast<NFAOrNode *>(cur);
    if (orNode) // an orNode will never have a "next" set. thus we don't have to worry about problems with
    {           // quantifying an or node.
      NFANode * one = orNode->one, * two = orNode->two;
      while (one->next) one = one->next;
      while (two->next) two = two->next;
      one->next = two->next = cur = registerNode(new NFAAcceptNode);
    }
    if (next)
    {
      cur = cur->next = quantify(next);
    }
  }
  if (inParen) raiseError();
  else
  {
    if (inOr) cur = cur->next = registerNode(new NFAAcceptNode);
    if (end) *end = cur;
  }

  flags = oldFlags;
  if (error) return NULL;

  return start;
}

Pattern * Pattern::compile(const std::string & pattern, const unsigned long mode)
{
  Pattern * p = new Pattern(pattern);
  NFANode * end;

  p->flags = mode;
  if ((mode & Pattern::LITERAL) != 0)
  {
    p->head = p->registerNode(new NFAStartNode);
    if ((mode & Pattern::CASE_INSENSITIVE) != 0)  p->head->next = p->registerNode(new NFACIQuoteNode(pattern));
    else                                          p->head->next = p->registerNode(new NFAQuoteNode(pattern));
    p->head->next->next = p->registerNode(new NFAEndNode);
  }
  else
  {
    p->head = p->parse(0, 0, &end);
    if (!p->head)
    {
      delete p;
      p = NULL;
    }
    else
    {
      if (!(p->head && p->head->isStartOfInputNode()))
      {
        NFANode * n = p->registerNode(new NFAStartNode);
        n->next = p->head;
        p->head = n;
      }
      end->next = p->registerNode(new NFAEndNode);
    }
  }
  if (p != NULL)
  {
    p->matcher = new Matcher(p, "");
  }

  return p;
}

Pattern * Pattern::compileAndKeep(const std::string & pattern, const unsigned long mode)
{
  Pattern * ret = NULL;
  std::map<std::string, Pattern*>::iterator it = compiledPatterns.find(pattern);

  if (it != compiledPatterns.end())
  {
    ret = it->second;
  }
  else
  {
    ret = compile(pattern, mode);
    compiledPatterns[pattern] = ret;
  }

  return ret;
}
std::string Pattern::replace(const std::string & pattern, const std::string & str,
                             const std::string & replacementText, const unsigned long mode)
{
  std::string ret;
  Pattern * p = Pattern::compile(pattern, mode);
  if (p)
  {
    ret = p->replace(str, replacementText);
    delete p;
  }
  return ret;
}

std::vector<std::string> Pattern::split(const std::string & pattern, const std::string & str, const bool keepEmptys,
                              const unsigned long limit, const unsigned long mode)
{
  std::vector<std::string> ret;
  Pattern * p = Pattern::compile(pattern, mode);
  if (p)
  {
    ret = p->split(str, keepEmptys, limit);
    delete p;
  }
  return ret;
}

std::vector<std::string> Pattern::findAll(const std::string & pattern, const std::string & str, const unsigned long mode)
{
  std::vector<std::string> ret;
  Pattern * p = Pattern::compile(pattern, mode);
  if (p)
  {
    ret = p->findAll(str);
    delete p;
  }
  return ret;
}

bool Pattern::matches(const std::string & pattern, const std::string & str, const unsigned long mode)
{
  bool ret = 0;
  Pattern * p = compile(pattern, mode);

  if (p)
  {
    ret = p->matches(str);
    delete p;
  }

  return ret;
}

bool Pattern::registerPattern(const std::string & name, const std::string & pattern, const unsigned long mode)
{
  Pattern * p = Pattern::compile(pattern, mode);
  if (!p) return 0;
  Pattern::registeredPatterns[name] = std::make_pair(pattern, mode);
  delete p;
  return 1;
}

void Pattern::unregisterPatterns()
{
  registeredPatterns.clear();
}
void Pattern::clearPatternCache()
{
  std::map<std::string, Pattern*>::iterator it;
  for (it = compiledPatterns.begin(); it != compiledPatterns.end(); ++it)
  {
    delete it->second;
  }
  compiledPatterns.clear();
}

std::pair<std::string, int>  Pattern::findNthMatch(const std::string & pattern, const std::string & str,
                                         const int matchNum, const unsigned long mode)
{
  std::pair<std::string, int> ret;
  Pattern * p = Pattern::compile(pattern, mode);

  ret.second = -1;
  if (p)
  {
    int i = -1;
    p->matcher->setString(str);
    while (i < matchNum && p->matcher->findNextMatch()) { ++i; }
    if (i == matchNum && p->matcher->getStartingIndex() >= 0)
    {
      ret.first = p->matcher->getGroup(0);
      ret.second = p->matcher->getStartingIndex();
    }
    delete p;
  }

  return ret;
}

Pattern::~Pattern()
{
  if (matcher) delete matcher;
  for (std::map<NFANode*, bool>::iterator it = nodes.begin(); it != nodes.end(); ++it)
  {
    delete it->first;
  }
}
std::string Pattern::replace(const std::string & str, const std::string & replacementText)
{
  int li = 0;
  std::string ret = "";

  matcher->setString(str);
  while (matcher->findNextMatch())
  {
    ret += str.substr(li, matcher->getStartingIndex() - li);
    ret += matcher->replaceWithGroups(replacementText);
    li = matcher->getEndingIndex();
  }
  ret += str.substr(li);

  return ret;
}
std::vector<std::string> Pattern::split(const std::string & str, const bool keepEmptys, const unsigned long limit)
{
  unsigned long lim = (limit == 0 ? MAX_QMATCH : limit);
  int li = 0;
  std::vector<std::string> ret;

  matcher->setString(str);

  while (matcher->findNextMatch() && ret.size() < lim)
  {
    if (matcher->getStartingIndex() == 0 && keepEmptys) ret.push_back("");
    if ((matcher->getStartingIndex() != matcher->getEndingIndex()) || keepEmptys)
    {
      if (li != matcher->getStartingIndex() || keepEmptys)
      {
        ret.push_back(str.substr(li, matcher->getStartingIndex() - li));
      }
      li = matcher->getEndingIndex();
    }
  }
  if (li < (int)str.size()) ret.push_back(str.substr(li));

  return ret;
}
std::vector<std::string> Pattern::findAll(const std::string & str)
{
  matcher->setString(str);
  return matcher->findAll();
}
bool Pattern::matches(const std::string & str)
{
  matcher->setString(str);
  return matcher->matches();
}
unsigned long Pattern::getFlags() const
{
  return flags;
}
std::string Pattern::getPattern() const
{
  return pattern;
}
Matcher * Pattern::createMatcher(const std::string & str)
{
  return new Matcher(this, str);
}
void Pattern::print()
{
  printf("Pattern(%s):\n", pattern.c_str());
  if (head) head->print(1);
}







static char * getPrintChar(const char ch)
{
  static char buf[1024];
  if      (ch == '0')   strcpy(buf, "\\0");
  else if (ch == '\r')  strcpy(buf, "\\r");
  else if (ch == '\t')  strcpy(buf, "\\t");
  else if (ch == '\n')  strcpy(buf, "\\n");
  else if (ch == 0x0C)  strcpy(buf, "\\p");
  else if (ch < ' ' || ch >= 0x80) sprintf(buf, "0x%02X", ch);
  else                  sprintf(buf, "%c", ch);
  return buf;
}
static void appendRawToClass(const int ch, char * const buf, int & curInd)
{
  switch (ch)
  {
  case '(':
  case ')':
  case '\\':
  case '-':
  case '[':
  case ']':
    buf[curInd++] = '\\';
  default:;
    buf[curInd++] = (char)ch;
  }
}
static void appendToClass0(const int j, int & start, const bool raw, const bool allowRanges, char * const buf, int & curInd)
{
  if (start != -1)
  {
    if (start + 1 == j)
    {
      if (raw)
      {
        appendRawToClass(start, buf, curInd);
      }
      else
      {
        sprintf(buf + curInd, "\\x%02X", start);
        curInd += 4;
      }
    }
    else if (start + 2 == j)
    {
      if (raw)
      {
        appendRawToClass(start, buf, curInd);
        appendRawToClass(start + 1, buf, curInd);
      }
      else
      {
        sprintf(buf + curInd, "\\x%02X\\x%02X", start, start + 1);
        curInd += 8;
      }
    }
    else
    {
      if (allowRanges)
      {
        if (raw)
        {
          appendRawToClass(start, buf, curInd);
          buf[curInd++] = '-';
          appendRawToClass(j - 1, buf, curInd);
        }
        else
        {
          sprintf(buf + curInd, "\\x%02X-\\x%02X", start, j - 1);
          curInd += 9;
        }
      }
      else
      {
        for (int k = start; k < j; ++k)
        {
          appendRawToClass(k, buf, curInd);
        }
      }
    }
  }
  start = -1;
}
static void appendToClass(const std::map<char, bool> & chars, const int * const range, const bool raw, const bool allowRanges, char * const buf, int & curInd)
{
  int start = -1, j;
  for (j = range[0]; j < range[1]; ++j)
  {
    if (chars.find((char)j) != chars.end())
    {
      if (start == -1) start = j;
    }
    else
    {
      appendToClass0(j, start, raw, allowRanges, buf, curInd);
    }
  }
  appendToClass0(j, start, raw, allowRanges, buf, curInd);
}
static char * getClassDesc(const std::map<char, bool> & chars, const bool inv, const bool firstPass = true)
{
  static char buf[1024];
  int curInd = 0;
  bool ok[256] = { false };

  buf[curInd++] = '[';
  if (inv)
  {
    buf[curInd++] = '^';
  }
  // check for lower case letters
  if (chars.find(' ')  != chars.end()) { buf[curInd++] = ' '; }
  if (chars.find(0x00) != chars.end()) { strcpy(buf + curInd, "\\0"); curInd += 2; }
  if (chars.find('\t') != chars.end()) { strcpy(buf + curInd, "\\t"); curInd += 2; }
  if (chars.find('\r') != chars.end()) { strcpy(buf + curInd, "\\r"); curInd += 2; }
  if (chars.find('\n') != chars.end()) { strcpy(buf + curInd, "\\n"); curInd += 2; }
  if (chars.find(0x0c) != chars.end()) { strcpy(buf + curInd, "\\p"); curInd += 2; }
  const int HEX_RANGES[][2] =
  {
    { 0x01, 0x09 },
    { 0x0B, 0x0B },
    { 0x0E, 0x20 },
    { 0x80, 0x100 },
  };
  const int RAW_RANGES[][2] =
  {
    { 0x20,     '0'     },
    { '0',      '9' + 1 },
    { '9' + 1,  'A'     },
    { 'A',      'Z' + 1 },
    { 'Z' + 1,  'a'     },
    { 'a',      'z' + 1 },
    { 'z' + 1,  0x80    },
  };
  const bool ALLOW_RANGES[] =
  {
    false,
    true,
    false,
    true,
    false,
    true,
    false,
  };
  const int NUM_HEX = sizeof(HEX_RANGES) / sizeof(HEX_RANGES[0]);
  const int NUM_RAW = sizeof(RAW_RANGES) / sizeof(RAW_RANGES[0]);

  for (int i = 0; i < NUM_HEX; ++i)
  {
    appendToClass(chars, HEX_RANGES[i], false, true, buf, curInd);
  }
  for (int i = 0; i < NUM_RAW; ++i)
  {
    appendToClass(chars, RAW_RANGES[i], true, ALLOW_RANGES[i], buf, curInd);
  }
  buf[curInd++] = ']';
  buf[curInd++] = 0;
  if (firstPass)
  {
    char newBuf[1024];
    std::map<char, bool> invChars;
    for (int i = 0; i < 256; ++i)
    {
      if (chars.find((char)i) == chars.end()) invChars[(char)i] = true;
    }
    strcpy(newBuf, buf);
    getClassDesc(invChars, !inv, false);
    if (strlen(buf) > strlen(newBuf)) strcpy(buf, newBuf);
  }
  return buf;
}

// NFANode

NFANode::NFANode() { next = NULL; }
NFANode::~NFANode() { }
void NFANode::findAllNodes(std::map<NFANode*, bool> & soFar)
{
  if (soFar.find(this) == soFar.end()) return;
  soFar[this] = 1;
  if (next) next->findAllNodes(soFar);
}
void NFANode::print(const int indent)
{
  printf("%*c%s\n", indent * 2, ' ', typeid(*this).name());
  if (next) next->print(indent);
}

// NFACharNode

NFACharNode::NFACharNode(const char c) { ch = c; }
int NFACharNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd < (int)str.size() && str[curInd] == ch) return next->match(str, matcher, curInd + 1, depth + 1);
  return -1;
}
void NFACharNode::print(const int indent)
{
  printf("%*c%s(0x%02X)\n", indent * 2, ' ', typeid(*this).name(), ch);
  if (next) next->print(indent);
}

// NFACICharNode

NFACICharNode::NFACICharNode(const char c) { ch = tolower(c); }
int NFACICharNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd < (int)str.size() && tolower(str[curInd]) == ch) return next->match(str, matcher, curInd + 1, depth + 1);
  return -1;
}
void NFACICharNode::print(const int indent)
{
  printf("%*c%s(0x%02X)\n", indent * 2, ' ', typeid(*this).name(), ch);
  if (next) next->print(indent);
}

// NFAStartNode

NFAStartNode::NFAStartNode() { }
int NFAStartNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int ret = -1, ci = curInd;

  matcher->starts[0] = curInd;
  if ((matcher->getFlags() & Matcher::MATCH_ENTIRE_STRING) == (unsigned int)Matcher::MATCH_ENTIRE_STRING)
  {
    if (curInd != 0)
    {
      matcher->starts[0] = -1;
      return -1;
    }
    return next->match(str, matcher, 0, depth + 1);
  }
  while ((ret = next->match(str, matcher, ci)) == -1 && ci < (int)str.size())
  {
    matcher->clearGroups();
    matcher->starts[0] = ++ci;
  }
  if (ret < 0) matcher->starts[0] = -1;
  return ret;
}

// NFAEndNode

NFAEndNode::NFAEndNode() { }
int NFAEndNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  matcher->ends[0] = curInd;
  if ((matcher->getFlags() & Matcher::MATCH_ENTIRE_STRING) != 0)
  {
    if (curInd == (int)str.size()) return curInd;
    matcher->ends[0] = -1;
    return -1;
  }
  return curInd;
}

// NFAQuantifierNode

void NFAQuantifierNode::findAllNodes(std::map<NFANode*, bool> & soFar)
{
  inner->findAllNodes(soFar);
  NFANode::findAllNodes(soFar);
}
NFAQuantifierNode::NFAQuantifierNode(Pattern * pat, NFANode * internal, const int minMatch, const int maxMatch)
{
  inner = internal;
  inner->next = pat->registerNode(new NFAAcceptNode);
  min = (minMatch < Pattern::MIN_QMATCH) ? Pattern::MIN_QMATCH : minMatch;
  max = (maxMatch > Pattern::MAX_QMATCH) ? Pattern::MAX_QMATCH : maxMatch;
}

int NFAQuantifierNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int i0, i1, i2 = 0;

  i0 = i1 = curInd;
  while (i2 < min)
  {
    ++i2;
    i1 = inner->match(str, matcher, i0, depth + 1);
    if (i1 <= i0) return i1; // i1 < i0 means i1 is -1
    i0 = i1;
  }

  return i1;
}
void NFAQuantifierNode::print(const int indent)
{
  printf("%*c%s(min=%d, max=%d)\n", indent * 2, ' ', typeid(*this).name(), min, max);
  printf("%*cinner\n", (indent + 1) * 2, ' ');
  if (inner) inner->print(indent + 2);
  if (next) next->print(indent);
}

// NFAGreedyQuantifierNode
NFAGreedyQuantifierNode::NFAGreedyQuantifierNode(Pattern * pat, NFANode * internal, const int minMatch, const int maxMatch)
                        : NFAQuantifierNode(pat, internal, minMatch, maxMatch) { }
int NFAGreedyQuantifierNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int t = NFAQuantifierNode::match(str, matcher, curInd, depth + 1);
  if (t != -1) return matchInternal(str, matcher, t, min, depth + 1);
  return t;
}
int NFAGreedyQuantifierNode::matchInternal(const std::string & str, Matcher * matcher, const int curInd, const int soFar, const int depth) const
{
  if (soFar >= max) return next->match(str, matcher, curInd, depth + 1);

  int i, j;

  i = inner->match(str, matcher, curInd, depth + 1);
  if (i != -1)
  {
    j = matchInternal(str, matcher, i, soFar + 1, depth + 1);
    if (j != -1) return j;
  }
  return next->match(str, matcher, curInd, depth + 1);
}

// NFALazyQuantifierNode

NFALazyQuantifierNode::NFALazyQuantifierNode(Pattern * pat, NFANode * internal, const int minMatch, const int maxMatch)
                      : NFAQuantifierNode(pat, internal, minMatch, maxMatch) { }
int NFALazyQuantifierNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int i, j, m = NFAQuantifierNode::match(str, matcher, curInd, depth + 1);

  if (m == -1) return -1;

  for (i = min; i < max; ++i)
  {
    j = next->match(str, matcher, m, depth + 1);
    if (j == -1)
    {
      j = inner->match(str, matcher, m, depth + 1);
      // if j < m, then j is -1, so we bail.
      // if j == m, then we would just go and call next->match on the same index,
      // but it already failed trying to match right there, so we know we can
      // just bail
      if (j <= m) return -1;
      m = j;
    }
    else return j;
  }
  return next->match(str, matcher, m, depth + 1);
}

// NFAPossessiveQuantifierNode

NFAPossessiveQuantifierNode::NFAPossessiveQuantifierNode(Pattern * pat, NFANode * internal, const int minMatch, const int maxMatch)
                            : NFAQuantifierNode(pat, internal, minMatch, maxMatch) { }
int NFAPossessiveQuantifierNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int i, j, m = NFAQuantifierNode::match(str, matcher, curInd, depth + 1);

  if (m == -1) return -1;
  for (i = min; i < max; ++i)
  {
    j = inner->match(str, matcher, m, depth + 1);
    if (j <= m) return next->match(str, matcher, m, depth + 1);
    m = j;
  }
  return next->match(str, matcher, m, depth + 1);
}

// NFAAcceptNode

NFAAcceptNode::NFAAcceptNode() { }
int NFAAcceptNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (!next) return curInd;
  else return next->match(str, matcher, curInd, depth + 1);
}

// NFAClassNode

NFAClassNode::NFAClassNode(const bool invert)
{
  inv = invert;
}
NFAClassNode::NFAClassNode(const std::string & clazz, const bool invert)
{
  inv = invert;
  for (int i = 0; i < (int)clazz.size(); ++i) vals[clazz[i]] = 1;
}
int NFAClassNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd < (int)str.size() && ((vals.find(str[curInd]) != vals.end()) ^ inv))
  {
    return next->match(str, matcher, curInd + 1, depth + 1);
  }
  return -1;
}
void NFAClassNode::print(const int indent)
{
  printf("%*c%s(%d vals, inv=%s)\n", indent * 2, ' ', typeid(*this).name(), (int)vals.size(), inv ? "true" : "false");
  if (next) next->print(indent);
}

// NFACIClassNode

NFACIClassNode::NFACIClassNode(const bool invert)
{
  inv = invert;
}
NFACIClassNode::NFACIClassNode(const std::string & clazz, const bool invert)
{
  inv = invert;
  for (int i = 0; i < (int)clazz.size(); ++i) vals[tolower(clazz[i])] = 1;
}
int NFACIClassNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd < (int)str.size() && ((vals.find(tolower(str[curInd])) != vals.end()) ^ inv))
  {
    return next->match(str, matcher, curInd + 1, depth + 1);
  }
  return -1;
}
void NFACIClassNode::print(const int indent)
{
  if (next) next->print(indent);
}

#undef to_lower

// NFASubStartNode

NFASubStartNode::NFASubStartNode() { }
int NFASubStartNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  return next->match(str, matcher, curInd, depth + 1);
}

// NFAOrNode

NFAOrNode::NFAOrNode(NFANode * first, NFANode * second) : one(first), two(second) { }
void NFAOrNode::findAllNodes(std::map<NFANode*, bool> & soFar)
{
  if (one) one->findAllNodes(soFar);
  if (two) two->findAllNodes(soFar);
  NFANode::findAllNodes(soFar);
}
int NFAOrNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int ci = one->match(str, matcher, curInd, depth + 1);
  if (ci != -1) return ci;
  return two->match(str, matcher, curInd, depth + 1);
/*
  int ci = one->match(str, matcher, curInd, depth + 1);
  if (ci != -1)
  {
    ci = next->match(str, matcher, ci, depth + 1);
    if (ci != -1) return ci;
  }
  ci = two->match(str, matcher, curInd, depth + 1);
  if (ci != -1) ci = next->match(str, matcher, ci, depth + 1);
  return -1;
*/
}
void NFAOrNode::print(const int indent)
{
  printf("%*c%s\n", indent * 2, ' ', typeid(*this).name());
  printf("%*cone\n", (indent + 1) * 2, ' ');
  one->print(indent + 2);
  printf("%*ctwo\n", (indent + 1) * 2, ' ');
  two->print(indent + 2);
}

// NFAQuoteNode

NFAQuoteNode::NFAQuoteNode(const std::string & quoted) : qStr(quoted) { }
int NFAQuoteNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd + qStr.size() > str.size())       return -1;
  if (str.substr(curInd, qStr.size()) != qStr) return -1;
  return next->match(str, matcher, curInd + qStr.size(), depth + 1);
}
void NFAQuoteNode::print(const int indent)
{
  printf("%*c%s(%s)\n", indent * 2, ' ', typeid(*this).name(), qStr.c_str());
  if (next) next->print(indent);
}

// NFACIQuoteNode

NFACIQuoteNode::NFACIQuoteNode(const std::string & quoted) : qStr(quoted) { }
int NFACIQuoteNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd + qStr.size() > str.size()) return -1;
  if (str_icmp(str.substr(curInd, qStr.size()).c_str(),  qStr.c_str())) return -1;
  return next->match(str, matcher, qStr.size(), depth + 1);
}
void NFACIQuoteNode::print(const int indent)
{
  printf("%*c%s(%s)\n", indent * 2, ' ', typeid(*this).name(), qStr.c_str());
  if (next) next->print(indent);
}

// NFALookAheadNode

NFALookAheadNode::NFALookAheadNode(NFANode * internal, const bool positive) : NFANode(), pos(positive), inner(internal) { }
void NFALookAheadNode::findAllNodes(std::map<NFANode*, bool> & soFar)
{
  if (inner) inner->findAllNodes(soFar);
  NFANode::findAllNodes(soFar);
}
int NFALookAheadNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  return ((inner->match(str, matcher, curInd, depth + 1) == -1) ^ pos) ? next->match(str, matcher, curInd, depth + 1) : -1;
}
void NFALookAheadNode::print(const int indent)
{
  printf("%*c%s(pos=%s)\n", indent * 2, ' ', typeid(*this).name(), pos ? "true" : "false");
  inner->print(indent + 1);
  if (next) next->print(indent);
}

// NFALookBehindNode

NFALookBehindNode::NFALookBehindNode(const std::string & str, const bool positive) : pos(positive), mStr(str) { }
int NFALookBehindNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (pos)
  {
    if (curInd < (int)mStr.size()) return -1;
    if (str.substr(curInd - mStr.size(), mStr.size()) == mStr) return next->match(str, matcher, curInd, depth + 1);
  }
  else
  {
    if (curInd < (int)mStr.size()) return next->match(str, matcher, curInd, depth + 1);
    if (str.substr(curInd - mStr.size(), mStr.size()) == mStr) return -1;
     return next->match(str, matcher, curInd, depth + 1);
  }
  return -1;
}
void NFALookBehindNode::print(const int indent)
{
  printf("%*c%s(str=%s, pos=%s)\n", indent * 2, ' ', typeid(*this).name(), mStr.c_str(), pos ? "true" : "false");
  if (next) next->print(indent);
}

// NFAStartOfLineNode

NFAStartOfLineNode::NFAStartOfLineNode() { }
int NFAStartOfLineNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd == 0 || str[curInd - 1] == '\n' || str[curInd - 1] == '\r')
  {
    return next->match(str, matcher, curInd, depth + 1);
  }
  return -1;
}

// NFAEndOfLineNode

NFAEndOfLineNode::NFAEndOfLineNode() { }
int NFAEndOfLineNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd >= (int)str.size() || str[curInd] == '\n' || str[curInd] == '\r')
  {
    return next->match(str, matcher, curInd, depth + 1);
  }
  return -1;
}

// NFAReferenceNode

NFAReferenceNode::NFAReferenceNode(const int groupIndex) : gi(groupIndex) { }
int NFAReferenceNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int len = matcher->ends[gi] - matcher->starts[gi];
  int ni = -1;
  if      (gi < 1 || matcher->ends[gi] < matcher->starts[gi] || len == 0)   ni = curInd;
  else if (curInd + len > (int)str.size())                                  return -1;
  else if (str.substr(curInd, len) != str.substr(matcher->starts[gi], len)) return -1;
  else                                                                      ni = curInd + len;

  return next->match(str, matcher, ni, depth + 1);
}
void NFAReferenceNode::print(const int indent)
{
  printf("%*c%s(gi=%d)\n", indent * 2, ' ', typeid(*this).name(), gi);
  if (next) next->print(indent);
}

// NFAStartOfInputNode

NFAStartOfInputNode::NFAStartOfInputNode() { }
int NFAStartOfInputNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd == 0) return next->match(str, matcher, curInd, depth + 1);
  return -1;
}

// NFAEndOfInputNode

NFAEndOfInputNode::NFAEndOfInputNode(const bool lookForTerm) : term(lookForTerm) { }
int NFAEndOfInputNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int len = (int)str.size();
  if      (curInd == len) return next->match(str, matcher, curInd, depth + 1);
  else if (term)
  {
    if      (curInd == len - 1 && (str[curInd] == '\r' || str[curInd] == '\n'))
    {
      return next->match(str, matcher, curInd, depth + 1);
    }
    else if (curInd == len - 2 && str.substr(curInd, 2) == "\r\n")
    {
      return next->match(str, matcher, curInd, depth + 1);
    }
  }
  return -1;
}
void NFAEndOfInputNode::print(const int indent)
{
  printf("%*c%s(term=%s)\n", indent * 2, ' ', typeid(*this).name(), term ? "true" : "false");
  if (next) next->print(indent);
}

// NFAWordBoundaryNode

NFAWordBoundaryNode::NFAWordBoundaryNode(const bool positive) : pos(positive) { }
int NFAWordBoundaryNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  #define is_alpha(x) (((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z'))

  int len = (int)str.size();
  bool ok = 0;
  char c1 = (curInd - 1 < len) ? str[curInd - 1] : -1;
  char c2 = (curInd     < len) ? str[curInd    ] : -1;

  if      (curInd == len) return next->match(str, matcher, curInd, depth + 1);
  if      (is_alpha(c1) ^ is_alpha(c2)) ok = 1;
  if (ok && pos) return next->match(str, matcher, curInd, depth + 1);
  return -1;

  #undef is_alpha
}
void NFAWordBoundaryNode::print(const int indent)
{
  printf("%*c%s(pos=%s)\n", indent * 2, ' ', typeid(*this).name(), pos ? "true" : "false");
  if (next) next->print(indent);
}

// NFAEndOfMatchNode

NFAEndOfMatchNode::NFAEndOfMatchNode() { }
int NFAEndOfMatchNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  if (curInd == matcher->lm) return next->match(str, matcher, curInd, depth + 1);
  return -1;
}

// NFAGroupHeadNode

NFAGroupHeadNode::NFAGroupHeadNode(const int groupIndex) : gi(groupIndex) { }
int NFAGroupHeadNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int ret, o = matcher->starts[gi];

  matcher->starts[gi] = curInd;
  ret = next->match(str, matcher, curInd, depth + 1);
  if (ret < 0) matcher->starts[gi] = o;

  return ret;
}
void NFAGroupHeadNode::print(const int indent)
{
  printf("%*c%s(gi=%d)\n", indent * 2, ' ', typeid(*this).name(), gi);
  if (next) next->print(indent);
}

// NFAGroupTailNode

NFAGroupTailNode::NFAGroupTailNode(const int groupIndex) : gi(groupIndex) { }
int NFAGroupTailNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int ret, o = matcher->ends[gi];
  matcher->ends[gi] = curInd;
  ret = next->match(str, matcher, curInd, depth + 1);
  if (ret < 0) matcher->ends[gi] = o;

  return ret;
}
void NFAGroupTailNode::print(const int indent)
{
  printf("%*c%s(gi=%d)\n", indent * 2, ' ', typeid(*this).name(), gi);
  if (next) next->print(indent);
}

// NFAGroupLoopPrologueNode

NFAGroupLoopPrologueNode::NFAGroupLoopPrologueNode(const int groupIndex) : gi(groupIndex) { }
int NFAGroupLoopPrologueNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int ret, o1 = matcher->groups[gi], o2 = matcher->groupPos[gi], o3 = matcher->groupIndeces[gi];

  matcher->groups[gi] = 0;
  matcher->groupPos[gi] = 0;
  matcher->groupIndeces[gi] = -1;
  ret = next->match(str, matcher, curInd, depth + 1);
  if (ret < 0)
  {
    matcher->groups[gi] = o1;
    matcher->groupPos[gi] = o2;
    matcher->groupIndeces[gi] = o3;
  }

  return ret;
}
void NFAGroupLoopPrologueNode::print(const int indent)
{
  printf("%*c%s(gi=%d)\n", indent * 2, ' ', typeid(*this).name(), gi);
  if (next) next->print(indent);
}

// NFAGroupLoopNode

NFAGroupLoopNode::NFAGroupLoopNode(NFANode * internal, const int minMatch, const int maxMatch,
                                   const int groupIndex, const int matchType)
{
  inner = internal;
  min = minMatch;
  max = maxMatch;
  gi = groupIndex;
  type = matchType;
}
void NFAGroupLoopNode::findAllNodes(std::map<NFANode*, bool> & soFar)
{
  if (inner) inner->findAllNodes(soFar);
  NFANode::findAllNodes(soFar);
}
int NFAGroupLoopNode::match(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  bool b = (curInd > matcher->groupIndeces[gi]);

  if (b && matcher->groups[gi] < min)
  {
    ++matcher->groups[gi];
    int o = matcher->groupIndeces[gi];
    matcher->groupIndeces[gi] = curInd;
    int ret = inner->match(str, matcher, curInd, depth + 1);
    if (ret < 0)
    {
      matcher->groupIndeces[gi] = o;
      --matcher->groups[gi];
    }
    return ret;
  }
  else if (!b || matcher->groups[gi] >= max)
  {
    return next->match(str, matcher, curInd, depth + 1);
  }
  else
  {
    switch (type)
    {
    case 0: return matchGreedy(str, matcher, curInd, depth + 1);
    case 1: return matchLazy(str, matcher, curInd, depth + 1);
    case 2: return matchPossessive(str, matcher, curInd, depth + 1);
    }
  }
  return -1;
}
int NFAGroupLoopNode::matchGreedy(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int o = matcher->groupIndeces[gi];            // save our info for backtracking
  matcher->groupIndeces[gi] = curInd;           // move along
  ++matcher->groups[gi];
  int ret = inner->match(str, matcher, curInd, depth + 1); // match internally
  if (ret < 0)
  {                                             // if we failed, then restore info and match next
    --matcher->groups[gi];
    matcher->groupIndeces[gi] = o;
    ret = next->match(str, matcher, curInd, depth + 1);
  }
  return ret;
}
int NFAGroupLoopNode::matchLazy(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int ret = next->match(str, matcher, curInd, depth + 1);  // be lazy, just go on
  if (ret < 0)
  {
    int o = matcher->groupIndeces[gi];          // save info for backtracking
    matcher->groupIndeces[gi] = curInd;         // advance our position
    ++matcher->groups[gi];
    ret = inner->match(str, matcher, curInd, depth + 1);   // match our internal stuff
    if (ret < 0)                                // if we failed, then restore the info
    {
      --matcher->groups[gi];
      matcher->groupIndeces[gi] = o;
    }
  }
  return ret;
}
int NFAGroupLoopNode::matchPossessive(const std::string & str, Matcher * matcher, const int curInd, const int depth) const
{
  int o = matcher->groupIndeces[gi];            // save info for backtracking
  matcher->groupPos[gi] = matcher->groups[gi];  // set a flag stating we have matcher at least this much
  matcher->groupIndeces[gi] = curInd;           // move along
  ++matcher->groups[gi];
  int ret = inner->match(str, matcher, curInd, depth + 1); // try and match again
  if (ret < 0)
  {                                             // if we fail, back off, but to an extent
    --matcher->groups[gi];
    matcher->groupIndeces[gi] = o;
    if (matcher->groups[gi] == matcher->groupPos[gi]) ret = next->match(str, matcher, curInd, depth + 1);
  }
  return ret;
}
void NFAGroupLoopNode::print(const int indent)
{
  printf("%*c%s(gi=%d, min=%d, max=%d, type=%s)\n", indent * 2, ' ', typeid(*this).name(), gi, min, max, type == 0 ? "greedy" : type == 1 ? "reluctant" : "possessive");
  if (next) next->print(indent);
}

#ifdef _WIN32
  #pragma warning(pop)
#endif
