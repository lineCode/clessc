#include "less/value/NumberValue.h"
#include "less/value/FunctionLibrary.h"

NumberValue::NumberValue(const Token& token) {
  tokens.push_back(token);

  switch (token.type) {
    case Token::NUMBER:
      type = NUMBER;
      break;
    case Token::PERCENTAGE:
      type = PERCENTAGE;
      break;
    case Token::DIMENSION:
      type = DIMENSION;
      break;
    default:
      throw new ValueException(
          "Token used for Value that isn't a "
          "number, percentage or dimension",
          *this->getTokens());
  }
}
NumberValue::NumberValue(double value) {
  tokens.push_back(Token("", Token::NUMBER, 0, 0, "generated"));
  type = NUMBER;
  setValue(value);
}
NumberValue::NumberValue(double value,
                         Token::Type type,
                         const std::string* unit) {
  if (type != Token::NUMBER && type != Token::PERCENTAGE &&
      type != Token::DIMENSION) {
    throw new ValueException(
        "Type can only be number, "
        "percentage or dimension.",
        *this->getTokens());
  }
  if (type == Token::DIMENSION && unit == NULL)
    throw new ValueException("Dimension requires a unit.", *this->getTokens());

  tokens.push_back(Token("", type, 0, 0, "generated"));

  switch (type) {
    case Token::NUMBER:
      this->type = NUMBER;
      break;
    case Token::PERCENTAGE:
      this->type = PERCENTAGE;
      break;
    case Token::DIMENSION:
      this->type = DIMENSION;
      break;
    default:
      break;
  }

  ostringstream stm;
  stm << setprecision(10) << value;
  if (type == Token::DIMENSION)
    stm << *unit;
  else if (type == Token::PERCENTAGE)
    stm << "%";
  tokens.front() = stm.str();
}

NumberValue::NumberValue(const NumberValue& n) {
  tokens.push_back(n.getTokens()->front());
  this->type = n.type;
}

NumberValue::~NumberValue() {
}

void NumberValue::verifyUnits(const NumberValue& n) {
  if (type == Value::DIMENSION && n.type == Value::DIMENSION &&
      getUnit().compare(n.getUnit()) != 0) {
    setValue(convert(n.getUnit()));
    setUnit(n.getUnit());
  }
}

double NumberValue::convert(const std::string& unit) const {
  UnitValue::UnitGroup group = UnitValue::getUnitGroup(unit);
  double value = getValue();

  if (UnitValue::getUnitGroup(getUnit()) == group) {
    switch (group) {
      case UnitValue::LENGTH:
        value = UnitValue::lengthToPx(value, getUnit());
        return UnitValue::pxToLength(value, unit);

      case UnitValue::TIME:
        value = UnitValue::timeToMs(value, getUnit());
        return UnitValue::msToTime(value, unit);

      case UnitValue::ANGLE:
        value = UnitValue::angleToRad(value, getUnit());
        return UnitValue::radToAngle(value, unit);

      default:
        return value;
    }
  } else
    throw new ValueException(
        "Can't do math on dimensions with "
        "different units.",
        *this->getTokens());
}

Value* NumberValue::add(const Value& v) const {
  const NumberValue* n;
  const StringValue* s;
  NumberValue* nret;
  StringValue* sret;

  if (isNumber(v)) {
    n = static_cast<const NumberValue*>(&v);
    nret = new NumberValue(getValue());
    if (type == NUMBER)
      nret->setType(*n);
    else {
      nret->setType(*this);
      nret->verifyUnits(*n);
    }
    nret->setValue(nret->getValue() + n->getValue());

    return nret;

  } else if (v.type == COLOR) {
    return static_cast<const Color*>(&v)->add(*this);

  } else if (v.type == STRING) {
    s = static_cast<const StringValue*>(&v);
    sret = new StringValue(*this, s->getQuotes());
    sret->append(v);
    return sret;

  } else {
    throw new ValueException("Unsupported type.", *this->getTokens());
  }
}
Value* NumberValue::substract(const Value& v) const {
  const NumberValue* n;
  NumberValue* ret;

  if (isNumber(v)) {
    n = static_cast<const NumberValue*>(&v);
    ret = new NumberValue(getValue());

    if (type == NUMBER)
      ret->setType(*n);
    else {
      ret->setType(*this);
      ret->verifyUnits(*n);
    }

    ret->setValue(ret->getValue() - n->getValue());

    return ret;
  } else
    throw new ValueException(
        "You can only substract a "
        "*number* from a number.",
        *this->getTokens());
}
Value* NumberValue::multiply(const Value& v) const {
  const NumberValue* n;
  NumberValue* ret;

  if (isNumber(v)) {
    n = static_cast<const NumberValue*>(&v);
    ret = new NumberValue(getValue());

    if (type == NUMBER)
      ret->setType(*n);
    else {
      ret->setType(*this);
      ret->verifyUnits(*n);
    }
    ret->setValue(ret->getValue() * n->getValue());
    return ret;

  } else if (v.type == COLOR) {
    return static_cast<const Color*>(&v)->multiply(*this);

  } else if (v.type == STRING) {
    return static_cast<const StringValue*>(&v)->multiply(*this);

  } else {
    throw new ValueException("Unsupported type.", *this->getTokens());
  }
}

Value* NumberValue::divide(const Value& v) const {
  const NumberValue* n;
  NumberValue* ret;

  if (isNumber(v)) {
    n = static_cast<const NumberValue*>(&v);
    ret = new NumberValue(getValue());

    if (type == NUMBER)
      ret->setType(*n);
    else {
      ret->setType(*this);
      ret->verifyUnits(*n);
    }
    ret->setValue(ret->getValue() / n->getValue());

    return ret;
  } else
    throw new ValueException(
        "You can only divide a number "
        "by a *number*.",
        *this->getTokens());
}

BooleanValue* NumberValue::equals(const Value& v) const {
  const NumberValue* n;
  const BooleanValue* b;

  if (isNumber(v)) {
    n = static_cast<const NumberValue*>(&v);
    return new BooleanValue(convert(n->getUnit()) == n->getValue());
  } else if (v.type == BOOLEAN) {
    // any number is falsy.
    b = static_cast<const BooleanValue*>(&v);
    return new BooleanValue(false == b->getValue());
  } else {
    throw new ValueException(
        "You can only compare a number "
        "with a *number*.",
        *this->getTokens());
  }
}

BooleanValue* NumberValue::lessThan(const Value& v) const {
  const NumberValue* n;
  const BooleanValue* b;

  if (isNumber(v)) {
    n = static_cast<const NumberValue*>(&v);
    return new BooleanValue(convert(n->getUnit()) < n->getValue());
  } else if (v.type == BOOLEAN) {
    b = static_cast<const BooleanValue*>(&v);
    return new BooleanValue(b->getValue());
  } else {
    throw new ValueException(
        "You can only compare a number "
        "with a *number*.",
        *this->getTokens());
  }
}

void NumberValue::setType(const NumberValue& n) {
  type = n.type;
  if (n.type == DIMENSION)
    setUnit(n.getUnit());
  else if (n.type == PERCENTAGE)
    tokens.front().type = Token::PERCENTAGE;
  else if (n.type == NUMBER) {
    setUnit("");
  }
}

double NumberValue::getValue() const {
  string number;
  istringstream stm;
  double ret;
  char c;

  for (unsigned int i = 0; i < tokens.front().size(); i++) {
    c = tokens.front()[i];
    if (!isdigit(c) && c != '.' && c != '-') {
      number = tokens.front().substr(0, i);
      break;
    }
  }
  if (number == "")
    number = tokens.front();
  stm.str(number);
  stm >> ret;
  return ret;
}
string NumberValue::getUnit() const {
  char c;
  unsigned int i;

  for (i = 0; i < tokens.front().size(); i++) {
    c = tokens.front()[i];
    if (!isdigit(c) && c != '.' && c != '-')
      return tokens.front().substr(i);
  }
  return string("");
}

void NumberValue::setUnit(string unit) {
  ostringstream stm;
  stm << setprecision(10) << getValue();
  stm << unit;
  tokens.front() = stm.str();

  if (unit.length() == 0) {
    type = NUMBER;
    tokens.front().type = Token::NUMBER;
  } else {
    type = DIMENSION;
    tokens.front().type = Token::DIMENSION;
  }
}

void NumberValue::setValue(double d) {
  ostringstream stm;
  stm << setprecision(10) << d;
  if (type == DIMENSION)
    stm << getUnit();
  else if (type == PERCENTAGE)
    stm << "%";
  tokens.front() = stm.str();
}

bool NumberValue::isNumber(const Value& val) {
  return (val.type == Value::NUMBER || val.type == Value::DIMENSION ||
          val.type == Value::PERCENTAGE);
}

void NumberValue::loadFunctions(FunctionLibrary& lib) {
  lib.push("unit", ".U?", &NumberValue::unit);
  lib.push("get-unit", ".", &NumberValue::get_unit);
  lib.push("isunit", "..", &NumberValue::is_unit);
  lib.push("ceil", ".", &NumberValue::ceil);
  lib.push("floor", ".", &NumberValue::floor);
  lib.push("percentage", "N", &NumberValue::percentage);
  lib.push("round", ".", &NumberValue::round);
  lib.push("sqrt", ".", &NumberValue::sqrt);
  lib.push("abs", ".", &NumberValue::abs);
  lib.push("sin", ".", &NumberValue::sin);
  lib.push("asin", "N", &NumberValue::asin);
  lib.push("cos", ".", &NumberValue::cos);
  lib.push("acos", "N", &NumberValue::acos);
  lib.push("tan", ".", &NumberValue::tan);
  lib.push("atan", "N", &NumberValue::atan);
  lib.push("pi", "", &NumberValue::pi);
  lib.push("pow", ".N", &NumberValue::pow);
  lib.push("mod", "..", &NumberValue::mod);
  lib.push("convert", "..", &NumberValue::convert);
  lib.push("min", "..+", &NumberValue::min);
  lib.push("max", "..+", &NumberValue::max);
  lib.push("isnumber", ".", &NumberValue::is_number);
  lib.push("isstring", ".", &NumberValue::is_string);
  lib.push("iscolor", ".", &NumberValue::is_color);
  lib.push("iskeyword", ".", &NumberValue::is_keyword);
  lib.push("isurl", ".", &NumberValue::is_url);
  lib.push("ispixel", ".", &NumberValue::is_pixel);
  lib.push("isem", ".", &NumberValue::is_em);
  lib.push("ispercentage", ".", &NumberValue::is_percentage);
}

// DIMENSION unit(DIMENSION, UNIT)
Value* NumberValue::unit(const vector<const Value*>& arguments) {
  NumberValue* ret;

  if (arguments[0]->type == Value::NUMBER ||
      arguments[0]->type == Value::DIMENSION) {
    ret = new NumberValue(((const NumberValue*)arguments[0])->getValue());

    if (arguments.size() > 1) {
      ret->setUnit(((const UnitValue*)arguments[1])->getUnit());
    } else
      ret->setUnit("");
    return ret;
  } else
    throw new ValueException(
        "argument 1 has to be a number "
        "or dimension",
        *arguments[0]->getTokens());
}

Value* NumberValue::get_unit(const vector<const Value*>& arguments) {
  Token t("", Token::IDENTIFIER, 0, 0, NULL);
  const NumberValue* val;
  if (arguments[0]->type == Value::NUMBER ||
      arguments[0]->type == Value::DIMENSION) {
    val = (const NumberValue*)arguments[0];
    t = val->getUnit();
    t.setLocation(val->getTokens()->front());
  }
  
  return new UnitValue(t);
}

Value* NumberValue::is_unit(const vector<const Value*>& arguments) {
  bool ret = false;
  
  if ((arguments[0]->type == Value::NUMBER ||
       arguments[0]->type == Value::DIMENSION) &&
      arguments[1]->type == Value::UNIT) {
    
    ret = (((const NumberValue*)arguments[0])->getUnit() ==
           ((const UnitValue*)arguments[1])->getUnit());
    
  } else if (arguments[0]->type == Value::PERCENTAGE &&
                arguments[1]->type == Value::STRING) {
    ret = (((const StringValue*)arguments[1])->getString() == "%");
  }

  return new BooleanValue(ret);
}


Value* NumberValue::ceil(const vector<const Value*>& args) {
  NumberValue* n;

  if (!NumberValue::isNumber(*args[0]))
    throw new ValueException(
        "ceil() only works on numeric "
        "values",
        *args[0]->getTokens());

  n = new NumberValue(*static_cast<const NumberValue*>(args[0]));

  double val = n->getValue();
  n->setValue(std::ceil(val));

  return n;
}
Value* NumberValue::floor(const vector<const Value*>& args) {
  NumberValue* n;

  if (!NumberValue::isNumber(*args[0]))
    throw new ValueException(
        "floor() only works on numeric "
        "values",
        *args[0]->getTokens());

  n = new NumberValue(*static_cast<const NumberValue*>(args[0]));

  double val = n->getValue();
  n->setValue(std::floor(val));
  return n;
}
Value* NumberValue::percentage(const vector<const Value*>& args) {
  const NumberValue* val = (const NumberValue*)args[0];
  return new NumberValue(val->getValue() * 100, Token::PERCENTAGE, NULL);
}

Value* NumberValue::round(const vector<const Value*>& args) {
  if (!NumberValue::isNumber(*args[0]))
    throw new ValueException(
        "round() only works on numeric "
        "values",
        *args[0]->getTokens());

  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);
  double val = n->getValue();
  double decimalplaces = 0;

  if (args.size() > 1)
    decimalplaces = ((const NumberValue*)args[1])->getValue();

  val = val * std::pow(10, decimalplaces);
  val = std::floor(val + 0.5);
  val = val / std::pow(10, decimalplaces);
  n->setValue(val);
  return n;
}
Value* NumberValue::sqrt(const vector<const Value*>& args) {
  if (!NumberValue::isNumber(*args[0]))
    throw new ValueException(
        "sqrt() only works on numeric "
        "values",
        *args[0]->getTokens());

  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);
  n->setValue(std::sqrt(n->getValue()));
  return n;
}
Value* NumberValue::abs(const vector<const Value*>& args) {
  if (!NumberValue::isNumber(*args[0]))
    throw new ValueException(
        "abs() only works on numeric "
        "values",
        *args[0]->getTokens());

  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);
  n->setValue(fabs(n->getValue()));
  return n;
}

Value* NumberValue::sin(const vector<const Value*>& args) {
  if (args[0]->type != Value::NUMBER && args[0]->type != Value::DIMENSION) {
    throw new ValueException(
        "sin() only works on numbers "
        "or dimensions",
        *args[0]->getTokens());
  }

  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);
  double val = n->getValue();
  std::string unit;

  if (n->type == Value::DIMENSION) {
    unit = n->getUnit();
    if (unit.compare("rad") != 0 && unit.compare("deg") != 0 &&
        unit.compare("grad") != 0 && unit.compare("turn") != 0) {
      throw new ValueException(
          "sin() requires rad, deg, "
          "grad or turn units.",
          *args[0]->getTokens());
    }
    val = UnitValue::angleToRad(val, unit);
  }

  n->setValue(std::sin(val));
  n->type = Value::NUMBER;
  n->setUnit("");
  return n;
}
Value* NumberValue::asin(const vector<const Value*>& args) {
  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);

  n->setValue(std::asin(n->getValue()));
  n->setUnit("rad");
  n->type = Value::DIMENSION;
  return n;
}
Value* NumberValue::cos(const vector<const Value*>& args) {
  if (args[0]->type != Value::NUMBER && args[0]->type != Value::DIMENSION) {
    throw new ValueException(
        "cos() only works on numbers "
        "or dimensions",
        *args[0]->getTokens());
  }
  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);
  double val = n->getValue();
  std::string unit;

  if (n->type == Value::DIMENSION) {
    unit = n->getUnit();
    if (unit.compare("rad") != 0 && unit.compare("deg") != 0 &&
        unit.compare("grad") != 0 && unit.compare("turn") != 0) {
      throw new ValueException(
          "cos() requires rad, deg, "
          "grad or turn units.",
          *args[0]->getTokens());
    }
    val = UnitValue::angleToRad(val, unit);
  }

  n->setValue(std::cos(val));
  n->type = Value::NUMBER;
  n->setUnit("");
  return n;
}
Value* NumberValue::acos(const vector<const Value*>& args) {
  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);

  n->setValue(std::acos(n->getValue()));
  n->setUnit("rad");
  n->type = Value::DIMENSION;
  return n;
}
Value* NumberValue::tan(const vector<const Value*>& args) {
  if (args[0]->type != Value::NUMBER && args[0]->type != Value::DIMENSION) {
    throw new ValueException(
        "tan() only works on numbers "
        "or dimensions",
        *args[0]->getTokens());
  }
  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);
  double val = n->getValue();
  std::string unit;

  if (n->type == Value::DIMENSION) {
    unit = n->getUnit();
    if (unit.compare("rad") != 0 && unit.compare("deg") != 0 &&
        unit.compare("grad") != 0 && unit.compare("turn") != 0) {
      throw new ValueException(
          "ta() requires rad, deg, "
          "grad or turn units.",
          *args[0]->getTokens());
    }
    val = UnitValue::angleToRad(val, unit);
  }

  n->setValue(std::tan(val));
  n->type = Value::NUMBER;
  n->setUnit("");
  return n;
}
Value* NumberValue::atan(const vector<const Value*>& args) {
  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);

  n->setValue(std::atan(n->getValue()));
  n->setUnit("rad");
  n->type = Value::DIMENSION;
  return n;
}
Value* NumberValue::pi(const vector<const Value*>& args) {
  (void)args;
  return new NumberValue(3.141592653589793);
}
Value* NumberValue::pow(const vector<const Value*>& args) {
  if (!NumberValue::isNumber(*args[0]))
    throw new ValueException("pow() only works on numeric values",
                             *args[0]->getTokens());

  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);
  double exp = ((const NumberValue*)args[1])->getValue();

  n->setValue(std::pow(n->getValue(), exp));
  return n;
}
Value* NumberValue::mod(const vector<const Value*>& args) {
  if (!NumberValue::isNumber(*args[0]) || !NumberValue::isNumber(*args[1]))
    throw new ValueException("mod() only works on numeric values",
                             *args[0]->getTokens());

  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);
  double val2 = ((NumberValue*)args[1])->getValue();

  n->setValue(std::fmod(n->getValue(), val2));
  return n;
}
Value* NumberValue::convert(const vector<const Value*>& args) {
  if (!NumberValue::isNumber(*args[0]))
    throw new ValueException("convert() only works on numeric values",
                             *args[0]->getTokens());
  if (args[1]->type != Value::STRING && args[1]->type != Value::UNIT) {
    throw new ValueException(
        "convert() requires a unit \
(or unit as a string)",
        *args[1]->getTokens());
  }

  NumberValue* n = new NumberValue(*(const NumberValue*)args[0]);
  std::string unit;

  if (args[1]->type == Value::STRING)
    unit = ((const StringValue*)args[1])->getString();
  else
    unit.append(((const UnitValue*)args[1])->getUnit());

  n->setValue(n->convert(unit));
  n->setUnit(unit);
  return n;
}

Value* NumberValue::min(const vector<const Value*>& arguments) {
  const NumberValue* min = NULL;
  vector<const Value *>::const_iterator it;
  
  for (it = arguments.begin(); it != arguments.end(); it++) {
    if (!NumberValue::isNumber(**it)) {
      throw new ValueException("arguments should be numbers",
                               *(*it)->getTokens());
    } else if (min == NULL) {
      min = (const NumberValue*)*it;
    } else {
      if (((const NumberValue*)*it)->convert(min->getUnit()) <
          min->getValue()) {
        min = (const NumberValue*)*it;
      }
    }
  }

  return new NumberValue(*min);
}

Value* NumberValue::max(const vector<const Value*>& arguments) {
  const NumberValue* max = NULL;
  vector<const Value *>::const_iterator it;
  
  for (it = arguments.begin(); it != arguments.end(); it++) {
    if (!NumberValue::isNumber(**it)) {
      throw new ValueException("arguments should be numbers",
                               *(*it)->getTokens());
    } else if (max == NULL) {
      max = (const NumberValue*)*it;
    } else {
      if (((const NumberValue*)*it)->convert(max->getUnit()) >
          max->getValue()) {
        max = (const NumberValue*)*it;
      }
    }
  }

  return new NumberValue(*max);
}

Value* NumberValue::is_number(const vector<const Value*>& arguments) {
  return new BooleanValue(NumberValue::isNumber(*arguments[0]));
}

Value* NumberValue::is_string(const vector<const Value*>& arguments) {
  return new BooleanValue(arguments[0]->type == Value::STRING &&
                          ((const StringValue*)arguments[0])
                          ->getQuotes() == true);
}

Value* NumberValue::is_color(const vector<const Value*>& arguments) {
  return new BooleanValue(arguments[0]->type == Value::COLOR);
}

Value* NumberValue::is_keyword(const vector<const Value*>& arguments) {
  return new BooleanValue(arguments[0]->type == Value::STRING &&
                          ((const StringValue*)arguments[0])
                          ->getQuotes() == false);
}

Value* NumberValue::is_url(const vector<const Value*>& arguments) {
  return new BooleanValue(arguments[0]->type == Value::URL);
}

Value* NumberValue::is_pixel(const vector<const Value*>& arguments) {
  return new BooleanValue(arguments[0]->type == Value::DIMENSION &&
                          ((const NumberValue*)arguments[0])
                          ->getUnit() == "px");
}

Value* NumberValue::is_em(const vector<const Value*>& arguments) {
  return new BooleanValue(arguments[0]->type == Value::DIMENSION &&
                          ((const NumberValue*)arguments[0])
                          ->getUnit() == "em");
}

Value* NumberValue::is_percentage(const vector<const Value*>& arguments) {
  return new BooleanValue(arguments[0]->type == Value::PERCENTAGE);
}
