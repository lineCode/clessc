#ifndef __less_lessstylesheet_Extension_h__
#define __less_lessstylesheet_Extension_h__

#include "less/stylesheet/Selector.h"

/**
 *
 */
class Extension {
private:
  Selector target;
  Selector extension;
  bool all;
  
public:
  Extension();
  virtual ~Extension();

  Selector& getTarget();
  void setTarget(const Selector& target);

  Selector& getExtension();
  void setExtension(const Selector& selector);

  void setAll(bool b);
  bool isAll() const;

  void updateSelector(Selector& s) const;
  void replaceInSelector(Selector& s) const;
};

#endif  // __less_lessstylesheet_Extension_h__
