use super::{walk, BinaryOperator, UnaryOperator};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Primitive {
    Nil,
    Boolean(bool),
    Integer(i64),
    Float(f64, i32),
    String(String),
}

#[derive(Clone, Debug, PartialEq)]
pub enum Expression<Expression_, Statement, Component> {
    Primitive(Primitive),
    Identifier(String),

    /* containers */
    Group(Box<Expression_>),
    Closure(Vec<Statement>),

    /* operations */
    UnaryOperation(UnaryOperator, Box<Expression_>),
    BinaryOperation(BinaryOperator, Box<Expression_>, Box<Expression_>),
    PropertyAccess(Box<Expression_>, String),
    FunctionCall(Box<Expression_>, Vec<Expression_>),

    /* domain syntax */
    Style(Vec<(String, Expression_)>),
    Component(Box<Component>),
}

impl<Visitor, Expression_, Statement, Component> walk::Walk<Visitor>
    for walk::Span<Expression<Expression_, Statement, Component>>
where
    Visitor: walk::Visit,
    Expression_: walk::Walk<Visitor, Output = Visitor::Expression>,
    Statement: walk::Walk<Visitor, Output = Visitor::Statement>,
    Component: walk::Walk<Visitor, Output = Visitor::Component>,
{
    type Output = Visitor::Expression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let Self(value, range) = self;

        match value {
            super::Expression::Primitive(x) => v.expression(super::Expression::Primitive(x), range),

            super::Expression::Identifier(x) => {
                v.expression(super::Expression::Identifier(x), range)
            }

            super::Expression::Group(x) => {
                let (x, v) = x.walk(v);

                v.expression(super::Expression::Group(Box::new(x)), range)
            }

            super::Expression::Closure(xs) => {
                let (xs, v) = xs.walk(v);

                v.expression(super::Expression::Closure(xs), range)
            }

            super::Expression::UnaryOperation(op, x) => {
                let (x, v) = x.walk(v);

                v.expression(super::Expression::UnaryOperation(op, Box::new(x)), range)
            }

            super::Expression::BinaryOperation(op, l, r) => {
                let (l, v) = l.walk(v);
                let (r, v) = r.walk(v);

                v.expression(
                    super::Expression::BinaryOperation(op, Box::new(l), Box::new(r)),
                    range,
                )
            }

            super::Expression::PropertyAccess(x, property) => {
                let (x, v) = x.walk(v);

                v.expression(
                    super::Expression::PropertyAccess(Box::new(x), property),
                    range,
                )
            }

            super::Expression::FunctionCall(x, arguments) => {
                let (x, v) = x.walk(v);
                let (arguments, v) = arguments.walk(v);

                v.expression(
                    super::Expression::FunctionCall(Box::new(x), arguments),
                    range,
                )
            }

            super::Expression::Component(x) => {
                let (x, v) = x.walk(v);

                v.expression(super::Expression::Component(Box::new(x)), range)
            }

            super::Expression::Style(xs) => {
                let (xs, v) = xs.walk(v);

                v.expression(super::Expression::Style(xs), range)
            }
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum Statement<Expression> {
    Expression(Expression),
    Variable(String, Expression),
}

impl<Visitor, Expression> walk::Walk<Visitor> for walk::Span<Statement<Expression>>
where
    Visitor: walk::Visit,
    Expression: walk::Walk<Visitor, Output = Visitor::Expression>,
{
    type Output = Visitor::Statement;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let Self(value, range) = self;

        match value {
            super::Statement::Expression(x) => {
                let (x, v) = x.walk(v);

                v.statement(super::Statement::Expression(x), range)
            }

            super::Statement::Variable(binding, x) => {
                let (x, v) = x.walk(v);

                v.statement(super::Statement::Variable(binding, x), range)
            }
        }
    }
}
