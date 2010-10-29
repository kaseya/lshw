/*
 * options.cc
 *
 * This module handles global options passed on the command-line.
 *
 */

#include "version.h"
#include "options.h"
#include "osutils.h"

#include <set>
#include <vector>
#include <string>
#include <map>

using namespace std;

__ID("@(#) $Id: options.cc 2069 2009-02-12 22:53:09Z lyonel $");

static set < string > disabled_tests;
static set < string > visible_classes;
static map < string, string > aliases;

void alias(const char * aname, const char * cname)
{
  aliases[lowercase(aname)] = lowercase(cname);
}


static string getcname(const char * aname)
{
  if(aliases.find(lowercase(aname)) != aliases.end())
    return lowercase(aliases[lowercase(aname)]);
  else
    return lowercase(aname);
}


static void remove_option_argument(int i,
int &argc,
char *argv[])
{
  for (int j = i; j + 2 < argc; j++)
    argv[j] = argv[j + 2];

  argc -= 2;
}


bool parse_options(int &argc,
char *argv[])
{
  int i = 1;
  string option = "";

  while (i < argc)
  {
    option = string(argv[i]);

    if (option == "-disable")
    {
      if (i + 1 >= argc)
        return false;                             // -disable requires an argument

      disable(argv[i + 1]);

      remove_option_argument(i, argc, argv);
    }
    else if (option == "-enable")
    {
      if (i + 1 >= argc)
        return false;                             // -enable requires an argument

      enable(argv[i + 1]);

      remove_option_argument(i, argc, argv);
    }
    else if ( (option == "-class") || (option == "-C") || (option == "-c"))
    {
      vector < string > classes;

      if (i + 1 >= argc)
        return false;                             // -class requires an argument

      splitlines(argv[i + 1], classes, ',');

      for (unsigned int j = 0; j < classes.size(); j++)
        visible_classes.insert(getcname(classes[j].c_str()));

      remove_option_argument(i, argc, argv);
    }
    else
      i++;
  }

  return true;
}


bool enabled(const char *option)
{
  return !(disabled(lowercase(option).c_str()));
}


bool disabled(const char *option)
{
  return disabled_tests.find(lowercase(option)) != disabled_tests.end();
}


void enable(const char *option)
{
  if (!disabled(option))
    return;

  disabled_tests.erase(lowercase(option));
}


void disable(const char *option)
{
  disabled_tests.insert(lowercase(option));
}


bool visible(const char *c)
{
  if (visible_classes.size() == 0)
    return true;
  return visible_classes.find(getcname(c)) != visible_classes.end();
}
