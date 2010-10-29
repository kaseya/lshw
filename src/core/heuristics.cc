/*
 * heuristics.cc
 *
 *
 */

#include "version.h"
#include "sysfs.h"
#include "osutils.h"

#include <regex.h>

__ID("@(#) $Id: heuristics.cc 2069 2009-02-12 22:53:09Z lyonel $");

string guessBusInfo(const string & info)
{
                                                  // 2.6-style PCI
  if(matches(info,"^[[:xdigit:]][[:xdigit:]][[:xdigit:]][[:xdigit:]]:[[:xdigit:]][[:xdigit:]]:[[:xdigit:]][[:xdigit:]]\\.[[:xdigit:]]$"))
  {
    return "pci@" + info;
  }
                                                  // 2.4-style PCI
  if(matches(info,"^[[:xdigit:]][[:xdigit:]]:[[:xdigit:]][[:xdigit:]]\\.[[:xdigit:]]$"))
  {
    return "pci@0000:" + info;
  }

                                                  // USB: host-port[.port]:config.interface
  if(matches(info, "^[0-9]+-[0-9]+(\\.[0-9]+)*:[0-9]+\\.[0-9]+$"))
  {
    size_t colon = info.rfind(":");
    size_t dash = info.find("-");

    return "usb@" + info.substr(0, dash) + ":" + info.substr(dash+1, colon-dash-1);
  }

  if(matches(info, "^[[:xdigit:]]+-[0-9]+$"))     // Firewire: guid-function
  {
    size_t dash = info.find("-");

    return "firewire@" + info.substr(0, dash);
  }

#if 1
//#ifdef __hppa__
  if(matches(info, "^[0-9]+(:[0-9]+)*$"))         // PA-RISC: x:y:z:t corresponds to /x/y/z/t
  {
    string result = "parisc@";

    for(unsigned i=0; i<info.length(); i++)
      if(info[i] == ':')
        result += '/';
    else
      result += info[i];
    return result;
  }
#endif
  return "";
}


static string guessParentBusInfo(const hwNode & child)
{
  string sysfs_path = sysfs_finddevice(child.getLogicalName());
  vector < string > path;
  string result = "";

  if(sysfs_path == "") return "";

  splitlines(sysfs_path, path, '/');

  if(path.size()>1)
    path.pop_back();
  else
    return "";

  while((result=="") && (path.size()>1))
  {
    result = guessBusInfo(path[path.size()-1]);
    path.pop_back();
  }
  return result;
}


hwNode * guessParent(const hwNode & child, hwNode & base)
{
  return base.findChildByBusInfo(guessParentBusInfo(child));
}

static const char *disk_manufacturers[] =
{
  "^ST.+", "Seagate",
  "^D...-.+", "IBM",
  "^IBM.+", "IBM",
  "^HITACHI.+", "Hitachi",
  "^IC.+", "Hitachi",
  "^HTS.+", "Hitachi",
  "^FUJITSU.+", "Fujitsu",
  "^MP.+", "Fujitsu",
  "^TOSHIBA.+", "Toshiba",
  "^MK.+", "Toshiba",
  "^MAXTOR.+", "Maxtor",
  "^Pioneer.+", "Pioneer",
  "^PHILIPS.+", "Philips",
  "^QUANTUM.+", "Quantum",
  "FIREBALL.+", "Quantum",
  "^WDC.+", "Western Digital",
  "WD.+", "Western Digital",
  NULL, NULL
};

bool guessVendor(hwNode & device)
{
  int i = 0;
  bool result = false;

  if(device.getVendor() != "")
    return false;

 
  if(device.getClass() == hw::disk)
    while (disk_manufacturers[i])
    {
      if (matches(device.getProduct().c_str(), disk_manufacturers[i], REG_ICASE))
      {
        device.setVendor(disk_manufacturers[i + 1]);
        result = true;
      }
      i += 2;
    }

  return result;
}

