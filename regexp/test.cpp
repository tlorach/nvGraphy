#include <regexp/Matcher.h>
#include <regexp/Pattern.h>
#include <regexp/WCMatcher.h>
#include <regexp/WCPattern.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include <wchar.h>

std::string escape(std::string str)
{
  for (int i = 0; i < (int)str.size(); ++i)
  {
    char replaceChar = 0;
    switch (str[i])
    {
    case '\n': replaceChar = 'n'; break;
    case '\r': replaceChar = 'r'; break;
    case '\t': replaceChar = 't'; break;
    }
    if (replaceChar)
    {
      std::string pre   = str.substr(0, i);
      std::string post  = str.substr(i + 1);
      str = pre + "\\" + replaceChar + post;
      i++;
    }
  }
  return str;
}

std::wstring wescape(std::wstring str)
{
  for (int i = 0; i < (int)str.size(); ++i)
  {
    wchar_t replaceChar = 0;
    switch (str[i])
    {
    case (wchar_t)'\n': replaceChar = (wchar_t)'n'; break;
    case (wchar_t)'\r': replaceChar = (wchar_t)'r'; break;
    case (wchar_t)'\t': replaceChar = (wchar_t)'t'; break;
    }
    if (replaceChar)
    {
      std::wstring pre   = str.substr(0, i);
      std::wstring post  = str.substr(i + 1);
      str = pre + L"\\" + replaceChar + post;
      i++;
    }
  }
  return str;
}

int main()
{

  {
    std::string pat0    = "abc(?i:abc)abc";
    std::string str0[]  = { "abcabcabc", "abcABCabc", "abcAbCabc", "abcabcABC", "abcABCABC", "abcAbCABC", };
    bool        exp0[]  = { true, true, true, false, false, false };
    int         num0    = 6;

    std::string pat1    = "abc.abc(?s:.)abc.abc";
    std::string str1[]  = { "abc abc\nabc abc", "abc\nabc\nabc abc" };
    bool        exp1[]  = { true, false };
    int         num1    = 2;

    std::string pat2    = "abc.abc(?is:.abc).abc";
    std::string str2[]  = { "abc abc\nABC abc", "abc abc\naBc abc", "abc\nabc\nAbC abc" };
    bool        exp2[]  = { true, true, false };
    int         num2    = 3;

    std::string pat3    = "abc.abc(?is).abc.(?-is)abc";
    std::string str3[]  = { "abc abc\nABC abc", "abc abc\naBc abc", "abc\nabc\nAbC abc" };
    bool        exp3[]  = { true, true, false };
    int         num3    = 3;

    std::string pat4    = "(?s)(?i)(?:^|.*[^a-zA-Z])BLUE(?:[^a-zA-Z].*|$)";
    std::string str4[]  = { "BLUE", "x BLUE", "BLUE x", "x BLUE x", "x blue x", "xbluex", "blah" };
    bool        exp4[]  = { true, true, true, true, true, false, false };
    int         num4    = 7;

    std::string   pats[] = { pat0, pat1, pat2, pat3, pat4 };
    std::string * strs[] = { str0, str1, str2, str3, str4 };
    bool        * exps[] = { exp0, exp1, exp2, exp3, exp4 };
    int           nums[] = { num0, num1, num2, num3, num4 };
    int numTests = sizeof(nums) / sizeof(nums[0]);

    printf("ASCII:\n");

    for (int i = 0; i < numTests; ++i)
    {
      printf("compiling %-60s", ("'" + pats[i] + "'").c_str());
      Pattern * p = Pattern::compile(pats[i]);
      if (!p)
      {
        printf("failed    [ error ]\n");
      }
      else
      {
        printf("passed    [ ok ]\n");
        for (int j = 0; j < nums[i]; ++j)
        {
          printf("matching  %-60s", ("'" + escape(strs[i][j]) + "'").c_str());
          bool b = p->matches(strs[i][j]);
          if (b) printf("success");
          else   printf("failure");
          if (b ^ exps[i][j]) printf("   [ error ]\n");
          else                printf("   [ ok ]\n");
        }
        delete p;
      }
    }
    printf("\n");
  }

/*
  {
    std::wstring pat0    = L"abc(?i:abc)abc";
    std::wstring str0[]  = { L"abcabcabc", L"abcABCabc", L"abcAbCabc", L"abcabcABC", L"abcABCABC", L"abcAbCABC", };
    bool         exp0[]  = { true, true, true, false, false, false };
    int          num0    = 6;

    std::wstring pat1    = L"abc.abc(?s:.)abc.abc";
    std::wstring str1[]  = { L"abc abc\nabc abc", L"abc\nabc\nabc abc" };
    bool         exp1[]  = { true, false };
    int          num1    = 2;

    std::wstring pat2    = L"abc.abc(?is:.abc).abc";
    std::wstring str2[]  = { L"abc abc\nABC abc", L"abc abc\naBc abc", L"abc\nabc\nAbC abc" };
    bool         exp2[]  = { true, true, false };
    int          num2    = 3;

    std::wstring pat3    = L"abc.abc(?is).abc.(?-is)abc";
    std::wstring str3[]  = { L"abc abc\nABC abc", L"abc abc\naBc abc", L"abc\nabc\nAbC abc" };
    bool         exp3[]  = { true, true, false };
    int          num3    = 3;

    std::wstring   pats[] = { pat0, pat1, pat2, pat3 };
    std::wstring * strs[] = { str0, str1, str2, str3 };
    bool         * exps[] = { exp0, exp1, exp2, exp3 };
    int            nums[] = { num0, num1, num2, num3 };
    int numTests = sizeof(nums) / sizeof(nums[0]);

    wprintf(L"Unicode:\n"); fflush(stdout);
    for (int i = 0; i < numTests; ++i)
    {
      wprintf(L"compiling %-60ls", (L"'" + wescape(pats[i]) + L"'").c_str());
      WCPattern * p = WCPattern::compile(pats[i]);
      if (!p)
      {
        wprintf(L"failed    [ error ]\n");
      }
      else
      {
        wprintf(L"passed    [ ok ]\n");
        for (int j = 0; j < nums[i]; ++j)
        {
          wprintf(L"matching  %-60ls", (L"'" + wescape(strs[i][j]) + L"'").c_str());
          bool b = p->matches(strs[i][j]);
          if (b) wprintf(L"success");
          else   wprintf(L"failure");
          if (b ^ exps[i][j]) wprintf(L"   [ error ]\n");
          else                wprintf(L"   [ ok ]\n");
        }
        delete p;
      }
    }
    printf("\n");
  }
*/
  exit(0);

  while (!feof(stdin))
  {
    Pattern * p;
    Matcher * m;
    char buf[1024];
    printf("enter a regex less than 1024 chars (hit CTRL+C to quit): ");
    scanf("%s", buf);
    p = Pattern::compile(buf);
    if (p)
    {
      printf("enter a file to parse (no spaces): ");
      scanf("%s", buf);
      FILE * fp = fopen(buf, "r");
      fseek(fp, 0, SEEK_END);
      long size = ftell(fp);
      printf("reading file of size %dkb\n", (int)(size / 1024)); fflush(stdout);
      char * arr = new char[size + 1];
      fseek(fp, 0, SEEK_SET);
      fread(arr, size + 1, 1, fp);
      fclose(fp);
      arr[size] = 0;
      printf("done\n"); fflush(stdout);
      m = p->createMatcher(arr);
      delete [] arr;
      printf("pattern matches entire file: %s\n", m->matches() ? "true" : "false");
      m->reset();
      printf("finding all matches inside file\n");
      while (m->findNextMatch())
      {
        std::vector<std::string> groups = m->getGroups(true);
        for (unsigned int i = 0; i < groups.size(); ++i)
        {
          printf("group %5d: %s\n", i, groups[i].c_str());
        }
      }

      delete m;
      delete p;
    }
    else
    {
      printf("regex syntax error\n");
    }
  }
  return 0;
}
