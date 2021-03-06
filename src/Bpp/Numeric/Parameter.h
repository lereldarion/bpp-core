//
// File: Parameter.h
// Authors:
//   Julien Dutheil
//   Francois Gindraud (2017)
// Created: 2003-10-15 15:40:47
// Last modified: 2017-07-10
//

/*
  Copyright or © or Copr. Bio++ Development Team, (November 17, 2004)

  This software is a computer program whose purpose is to provide classes
  for numerical calculus.

  This software is governed by the CeCILL license under French law and
  abiding by the rules of distribution of free software. You can use,
  modify and/ or redistribute the software under the terms of the CeCILL
  license as circulated by CEA, CNRS and INRIA at the following URL
  "http://www.cecill.info".

  As a counterpart to the access to the source code and rights to copy,
  modify and redistribute granted by the license, users are provided only
  with a limited warranty and the software's author, the holder of the
  economic rights, and the successive licensors have only limited
  liability.

  In this respect, the user's attention is drawn to the risks associated
  with loading, using, modifying and/or developing or reproducing the
  software by the user in light of its specific status of free software,
  that may mean that it is complicated to manipulate, and that also
  therefore means that it is reserved for developers and experienced
  professionals having in-depth computer knowledge. Users are therefore
  encouraged to load and test the software's suitability as regards their
  requirements in conditions enabling the security of their systems and/or
  data to be ensured and, more generally, to use and operate it in the
  same conditions as regards security.

  The fact that you are presently reading this means that you have had
  knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef BPP_NUMERIC_PARAMETER_H
#define BPP_NUMERIC_PARAMETER_H

#include "../Clonable.h"
#include "../Utils/Memory.h"
#include "Constraints.h" // TODO move to cpp ; requires moving to shared_ptr before

#include <string>
#include <vector>

namespace bpp
{
  // Forward declarations
  class Parameter;
  class Constraint;

  class ParameterEvent
  {
    // TODO use ref instead of pointer
  public:
    ParameterEvent(Parameter* parameter) noexcept;
    Parameter* getParameter() const noexcept { return parameter_; }

  private:
    Parameter* parameter_; // Will be copied
  };

  /** @brief The parameter listener interface.
   * Imlementing this interface allows to catch events associated to parameters modifications.
   * Listeners must have an identifier that will be used to pinpoint it when attached to a list.
   * This identifier needs not be unique though, but listeners with identical id will be undistinguishable.
   */
  class ParameterListener : public virtual Clonable
  {
  public:
    ParameterListener* clone() const override = 0;

    /// @return The identifier of this listener.
    virtual const std::string& getId() const = 0;

    /** @brief Notify a renaming action.
     * @param event Event associated to the action.
     */
    virtual void parameterNameChanged(ParameterEvent& event) = 0;

    /** @brief Notify a value change.
     * @param event Event associated to the action.
     */
    virtual void parameterValueChanged(ParameterEvent& event) = 0;
  };

  /** @brief This class is designed to facilitate the manipulation of parameters.
   * A parameter object contains a <i>value</i> stored as a double.
   * It also contains a <i>name</i> and optionaly a constraint.
   * Constraint objects allows to apply restriction on the value of the parameter,
   * for instance positive number, or a particular interval and so on.
   * @see ParameterList, Parametrizable, Constraint.
   */
  class Parameter : public virtual Clonable
  {
  protected:
    std::string name_;      // Parameter name
    double value_{0.0};     // Parameter value
    double precision_{0.0}; // Precision needed for Parameter value
    CopyUniquePtr<Constraint, ConditionalOwnershipPolicy<Constraint>> constraint_{nullptr}; // A constraint on the value
    std::vector<CopyUniquePtr<ParameterListener, ConditionalOwnershipPolicy<ParameterListener>>> listeners_;
    // TODO move to shared_ptr

  public: // Class constructors and destructors:
    /// @brief Default contructor. Creates a parameter with no name, no constraint, and a value of 0.
    Parameter();

    /** @brief Build a new parameter.
     * @param name       The parameter name.
     * @param value      The parameter value.
     * @param constraint A  pointer toward a constraint Object.
     * @param attachConstraint Tell if the constraint must be attached to this parameter, or shared
     * @param precision An optional parameter precision (default 0)
     * between different objects (the default behavior, for backward compatibility).
     * If the first case, the constraint object will be destroyed when the parameter is destroyed,
     * and duplicated when the parameter is copied.
     * @throw ConstraintException If the parameter value does not match the contraint.
     */
    Parameter(const std::string& name,
              double value,
              Constraint* constraint,
              bool attachConstraint,
              double precision = 0);

    /** @brief Build a new parameter.
     * @param name       The parameter name.
     * @param value      The parameter value.
     * @param constraint An optional pointer toward a constraint Object. The constraint will be copied and attached to this instance.
     * @param precision An optional parameter precision (default 0)
     * @throw ConstraintException If the parameter value does not match the contraint.
     */
    Parameter(const std::string& name, double value, const Constraint* constraint = 0, double precision = 0);

    virtual ~Parameter();

    Parameter* clone() const;

  public:
    /** @brief Set the name of this parameter.
     * @param name the new parameter name.
     */
    virtual void setName(const std::string& name);

    /** @brief Set the value of this parameter.
     * @param value the new parameter value.
     */
    virtual void setValue(double value);

    /** @brief Set the precision of this parameter.
     * @param precision the new parameter precision.
     */
    void setPrecision(double precision);

    /** @brief Get the name of this parameter.
     * @return The parameter name.
     */
    virtual const std::string& getName() const { return name_; }

    /** @brief Get the value of this parameter.
     * @return The parameter value.
     */
    virtual double getValue() const { return value_; }

    /** @brief Get the precision of this parameter.
     * @return The precision value.
     */
    virtual double getPrecision() const { return precision_; }

    /** @brief Return the constraint associated to this parameter if there is one.
     * @return A pointer toward the constraint, or NULL if there is no constraint.
     */
    virtual const Constraint* getConstraint() const;

    /** @brief Return the constraint associated to this parameter if there is one.
     * @return A pointer toward the constraint, or NULL if there is no constraint.
     */
    virtual Constraint* getConstraint();

    /// @return True if this parameter has a contraint.
    virtual bool hasConstraint() const;

    /** @brief Remove the constraint associated to this parameter.
     * Warning! The contraint objet is not deleted.
     * @return A pointer toward the formerly used contraint.
     */
    virtual Constraint* removeConstraint();

    /** @brief Set a constraint to this parameter.
     * @param constraint a pointer to the constraint (may be null)
     * @param attach says if the constraint is attached to the Parameter (default: false).
     */
    virtual void setConstraint(Constraint* constraint, bool attach = false);

    /** @brief Add a new listener to this parameter.
     * @param listener The listener to add.
     * @param attachListener Tell if the parameter will own this listener.
     * If so, deep copies will be made when cloning the parameter, and the listener will be destroyed upon
     * destruction of the parameter or upon removal. Alternatively, only superficial copies will be made,
     * and the listener will persist if the parameter is destroyed.
     */
    virtual void addParameterListener(ParameterListener* listener, bool attachListener = true);

    /** @brief Remove all listeners with a given id from this parameter.
     * @param listenerId The id of listener to remove.
     */
    virtual void removeParameterListener(const std::string& listenerId);

    /** @brief Tell is there is a listener with a given id from this parameter.
     * @param listenerId The id of listener to remove.
     * @return True if at list one listener with the given id was found.
     */
    virtual bool hasParameterListener(const std::string& listenerId);

  protected:
    void fireParameterNameChanged(ParameterEvent& event);
    void fireParameterValueChanged(ParameterEvent& event);

  public:
    static const IntervalConstraint R_PLUS;
    static const IntervalConstraint R_PLUS_STAR;
    static const IntervalConstraint R_MINUS;
    static const IntervalConstraint R_MINUS_STAR;
    static const IntervalConstraint PROP_CONSTRAINT_IN;
    static const IntervalConstraint PROP_CONSTRAINT_EX;
  };

} // end of namespace bpp.
#endif // BPP_NUMERIC_PARAMETER_H
