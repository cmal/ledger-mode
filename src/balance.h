#ifndef _BALANCE_H
#define _BALANCE_H

#include "amount.h"

namespace ledger {

// jww (2007-05-01): What really should be the operational semantics
// of a balance?

class balance_t
  : public ordered_field_operators<balance_t,
           ordered_field_operators<balance_t, amount_t> >
{
public:
  typedef std::map<const commodity_t *, amount_t>  amounts_map;
  typedef std::pair<const commodity_t *, amount_t> amounts_pair;

protected:
  amounts_map amounts;

  friend class value_t;
  friend class entry_base_t;

public:
  // constructors
  balance_t() {
    TRACE_CTOR(balance_t, "");
  }
  balance_t(const balance_t& bal) {
    TRACE_CTOR(balance_t, "copy");
    for (amounts_map::const_iterator i = bal.amounts.begin();
	 i != bal.amounts.end();
	 i++)
      *this += (*i).second;
  }
  balance_t(const amount_t& amt) {
    TRACE_CTOR(balance_t, "const amount_t&");
    amounts.insert(amounts_pair(&amt.commodity(), amt));
  }
  ~balance_t() {
    TRACE_DTOR(balance_t);
  }

  // assignment operator
  balance_t& operator=(const balance_t& bal) {
    if (this != &bal) {
      amounts.clear();
      for (amounts_map::const_iterator i = bal.amounts.begin();
	   i != bal.amounts.end();
	   i++)
	*this += (*i).second;
    }
    return *this;
  }

  int compare(const balance_t& bal) const;

  bool operator==(const balance_t& bal) const {
    amounts_map::const_iterator i, j;
    for (i = amounts.begin(), j = bal.amounts.begin();
	 i != amounts.end() && j != bal.amounts.end();
	 i++, j++) {
      if (! ((*i).first  == (*j).first &&
	     (*i).second == (*j).second))
	return false;
    }
    return i == amounts.end() && j == bal.amounts.end();
  }

  // in-place arithmetic
  balance_t& operator+=(const balance_t& bal) {
    for (amounts_map::const_iterator i = bal.amounts.begin();
	 i != bal.amounts.end();
	 i++)
      *this += (*i).second;
    return *this;
  }
  balance_t& operator-=(const balance_t& bal) {
    for (amounts_map::const_iterator i = bal.amounts.begin();
	 i != bal.amounts.end();
	 i++)
      *this -= (*i).second;
    return *this;
  }
  balance_t& operator*=(const balance_t& bal);
  balance_t& operator*=(const amount_t& amt); // special
  balance_t& operator/=(const balance_t& bal);
  balance_t& operator/=(const amount_t& amt); // special

  // comparison
  bool operator<(const balance_t& bal) const {
    for (amounts_map::const_iterator i = bal.amounts.begin();
	 i != bal.amounts.end();
	 i++) {
      optional<amount_t> amt = amount(*(*i).first);
      if (amt && ! (*amt < (*i).second))
	return false;
    }

    for (amounts_map::const_iterator i = amounts.begin();
	 i != amounts.end();
	 i++) {
      optional<amount_t> amt = bal.amount(*(*i).first);
      if (amt && ! ((*i).second < *amt))
	return false;
    }

    if (bal.amounts.size() == 0 && amounts.size() == 0)
      return false;

    return true;
  }

  // unary negation
  void in_place_negate() {
    for (amounts_map::iterator i = amounts.begin();
	 i != amounts.end();
	 i++)
      (*i).second = (*i).second.negate();
  }
  balance_t negate() const {
    balance_t temp = *this;
    temp.in_place_negate();
    return temp;
  }
  balance_t operator-() const {
    return negate();
  }

  // conversion operators
  operator bool() const {
    for (amounts_map::const_iterator i = amounts.begin();
	 i != amounts.end();
	 i++)
      if ((*i).second)
	return true;
    return false;
  }

  bool is_realzero() const {
    if (amounts.size() == 0)
      return true;
    for (amounts_map::const_iterator i = amounts.begin();
	 i != amounts.end();
	 i++)
      if (! (*i).second.is_realzero())
	return false;
    return true;
  }

  optional<amount_t>  amount(const optional<const commodity_t&>& commodity =
			     optional<const commodity_t&>()) const;
  optional<balance_t> value(const optional<moment_t>& moment =
			    optional<moment_t>()) const;

  balance_t
  strip_annotations(const bool keep_price = amount_t::keep_price,
		    const bool keep_date  = amount_t::keep_date,
		    const bool keep_tag   = amount_t::keep_tag) const;

  void write(std::ostream& out, const int first_width,
	     const int latter_width = -1) const;

  balance_t abs() const {
    balance_t temp = *this;
    for (amounts_map::iterator i = temp.amounts.begin();
	 i != temp.amounts.end();
	 i++)
      (*i).second = (*i).second.abs();
    return temp;
  }

  void in_place_reduce() {
    for (amounts_map::iterator i = amounts.begin();
	 i != amounts.end();
	 i++)
      (*i).second.in_place_reduce();
  }
  balance_t reduce() const {
    balance_t temp(*this);
    temp.in_place_reduce();
    return temp;
  }

  void in_place_round() {
    for (amounts_map::iterator i = amounts.begin();
	 i != amounts.end();
	 i++)
      (*i).second = (*i).second.round();
  }
  balance_t round() const {
    balance_t temp(*this);
    temp.in_place_round();
    return temp;
  }

  balance_t unround() const {
    balance_t temp;
    for (amounts_map::const_iterator i = amounts.begin();
	 i != amounts.end();
	 i++)
      if ((*i).second.commodity())
	temp += (*i).second.unround();
    return temp;
  }

  bool valid() const {
    for (amounts_map::const_iterator i = amounts.begin();
	 i != amounts.end();
	 i++)
      if (! (*i).second.valid())
	return false;
    return true;
  }
};

inline std::ostream& operator<<(std::ostream& out, const balance_t& bal) {
  bal.write(out, 12);
  return out;
}

class balance_pair_t
  : public ordered_field_operators<balance_pair_t,
           ordered_field_operators<balance_pair_t, balance_t,
           ordered_field_operators<balance_pair_t, amount_t> > >
{
  balance_t quantity;
  optional<balance_t> cost;

  friend class value_t;
  friend class entry_base_t;

public:
  // constructors
  balance_pair_t() {
    TRACE_CTOR(balance_pair_t, "");
  }
  balance_pair_t(const balance_pair_t& bal_pair)
    : quantity(bal_pair.quantity), cost(bal_pair.cost) {
    TRACE_CTOR(balance_pair_t, "copy");
  }
  balance_pair_t(const balance_t& _quantity)
    : quantity(_quantity) {
    TRACE_CTOR(balance_pair_t, "const balance_t&");
  }
  balance_pair_t(const amount_t& _quantity)
    : quantity(_quantity) {
    TRACE_CTOR(balance_pair_t, "const amount_t&");
  }
  ~balance_pair_t() {
    TRACE_DTOR(balance_pair_t);
  }

  // assignment operator
  balance_pair_t& operator=(const balance_pair_t& bal_pair) {
    if (this != &bal_pair) {
      quantity = bal_pair.quantity;
      cost     = bal_pair.cost;
    }
    return *this;
  }

  // in-place arithmetic
  balance_pair_t& operator+=(const balance_pair_t& bal_pair) {
    if (bal_pair.cost && ! cost)
      cost = quantity;
    quantity += bal_pair.quantity;
    if (cost)
      *cost += bal_pair.cost ? *bal_pair.cost : bal_pair.quantity;
    return *this;
  }
  balance_pair_t& operator-=(const balance_pair_t& bal_pair) {
    if (bal_pair.cost && ! cost)
      cost = quantity;
    quantity -= bal_pair.quantity;
    if (cost)
      *cost -= bal_pair.cost ? *bal_pair.cost : bal_pair.quantity;
    return *this;
  }
  balance_pair_t& operator*=(const balance_pair_t& bal_pair) {
    if (bal_pair.cost && ! cost)
      cost = quantity;
    quantity *= bal_pair.quantity;
    if (cost)
      *cost *= bal_pair.cost ? *bal_pair.cost : bal_pair.quantity;
    return *this;
  }
  balance_pair_t& operator/=(const balance_pair_t& bal_pair) {
    if (bal_pair.cost && ! cost)
      cost = quantity;
    quantity /= bal_pair.quantity;
    if (cost)
      *cost /= bal_pair.cost ? *bal_pair.cost : bal_pair.quantity;
    return *this;
  }

  // comparison
  bool operator==(const balance_pair_t& bal_pair) const {
    return quantity == bal_pair.quantity;
  }
  bool operator<(const balance_pair_t& bal_pair) const {
    return quantity < bal_pair.quantity;
  }

  // unary negation
  void in_place_negate() {
    quantity.in_place_negate();
    if (cost)
      cost->in_place_negate();
  }
  balance_pair_t negate() const {
    balance_pair_t temp = *this;
    temp.in_place_negate();
    return temp;
  }
  balance_pair_t operator-() const {
    return negate();
  }

  // test for non-zero (use ! for zero)
  operator bool() const {
    return quantity;
  }

  bool is_realzero() const {
    return ((! cost || cost->is_realzero()) && quantity.is_realzero());
  }

  balance_pair_t abs() const {
    balance_pair_t temp = *this;
    temp.quantity = temp.quantity.abs();
    if (temp.cost)
      temp.cost = temp.cost->abs();
    return temp;
  }

  optional<amount_t>  amount(const optional<const commodity_t&>& commodity =
			     optional<const commodity_t&>()) const {
    return quantity.amount(commodity);
  }
  optional<balance_t> value(const optional<moment_t>& moment =
			    optional<moment_t>()) const {
    return quantity.value(moment);
  }

  balance_t
  strip_annotations(const bool keep_price = amount_t::keep_price,
		    const bool keep_date  = amount_t::keep_date,
		    const bool keep_tag   = amount_t::keep_tag) const {
    return quantity.strip_annotations(keep_price, keep_date, keep_tag);
  }

  void write(std::ostream& out, const int first_width,
	     const int latter_width = -1) const {
    quantity.write(out, first_width, latter_width);
  }

  balance_pair_t& add(const amount_t&  amt,
		      const optional<amount_t>& a_cost = optional<amount_t>()) {
    if (a_cost && ! cost)
      cost = quantity;
    quantity += amt;
    if (cost)
      *cost += a_cost ? *a_cost : amt;
    return *this;
  }

  bool valid() {
    return quantity.valid() && (! cost || cost->valid());
  }

  void in_place_reduce() {
    quantity.in_place_reduce();
    if (cost) cost->in_place_reduce();
  }
  balance_pair_t reduce() const {
    balance_pair_t temp(*this);
    temp.in_place_reduce();
    return temp;
  }

  void in_place_round() {
    quantity = quantity.round();
    if (cost)
      cost = cost->round();
  }
  balance_pair_t round() const {
    balance_pair_t temp(*this);
    temp.in_place_round();
    return temp;
  }

  balance_pair_t unround() {
    balance_pair_t temp(quantity.unround());
    if (cost)
      temp.cost = cost->unround();
    return temp;
  }

  friend std::ostream& operator<<(std::ostream& out,
				  const balance_pair_t& bal_pair);
};

inline std::ostream& operator<<(std::ostream& out,
				const balance_pair_t& bal_pair) {
  bal_pair.quantity.write(out, 12);
  return out;
}

} // namespace ledger

#endif // _BALANCE_H
