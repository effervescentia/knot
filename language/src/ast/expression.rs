use super::{BinaryOperator, UnaryOperator};
use crate::walk::{Visit, Walk};
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

impl<Visitor, Context, Expression_, Statement, Component> Walk<Visitor, Context>
    for (Expression<Expression_, Statement, Component>, Context)
where
    Visitor: Visit<Context>,
    Expression_: Walk<Visitor, Context, Output = Visitor::Expression>,
    Statement: Walk<Visitor, Context, Output = Visitor::Statement>,
    Component: Walk<Visitor, Context, Output = Visitor::Component>,
{
    type Output = Visitor::Expression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            super::Expression::Primitive(x) => v.expression(super::Expression::Primitive(x), ctx),

            super::Expression::Identifier(x) => v.expression(super::Expression::Identifier(x), ctx),

            super::Expression::Group(x) => {
                let (x, v) = x.walk(v);

                v.expression(super::Expression::Group(Box::new(x)), ctx)
            }

            super::Expression::Closure(xs) => {
                let (xs, v) = v.scoped(|v| xs.walk(v));

                v.expression(super::Expression::Closure(xs), ctx)
            }

            super::Expression::UnaryOperation(op, x) => {
                let (x, v) = x.walk(v);

                v.expression(super::Expression::UnaryOperation(op, Box::new(x)), ctx)
            }

            super::Expression::BinaryOperation(op, l, r) => {
                let (l, v) = l.walk(v);
                let (r, v) = r.walk(v);

                v.expression(
                    super::Expression::BinaryOperation(op, Box::new(l), Box::new(r)),
                    ctx,
                )
            }

            super::Expression::PropertyAccess(x, property) => {
                let (x, v) = x.walk(v);

                v.expression(
                    super::Expression::PropertyAccess(Box::new(x), property),
                    ctx,
                )
            }

            super::Expression::FunctionCall(x, arguments) => {
                let (x, v) = x.walk(v);
                let (arguments, v) = arguments.walk(v);

                v.expression(super::Expression::FunctionCall(Box::new(x), arguments), ctx)
            }

            super::Expression::Component(x) => {
                let (x, v) = x.walk(v);

                v.expression(super::Expression::Component(Box::new(x)), ctx)
            }

            super::Expression::Style(xs) => {
                let (xs, v) = xs.walk(v);

                v.expression(super::Expression::Style(xs), ctx)
            }
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum Statement<Expression> {
    Expression(Expression),
    // TODO: change this String to Binding
    Variable(String, Expression),
}

impl<Visitor, Context, Expression> Walk<Visitor, Context> for (Statement<Expression>, Context)
where
    Visitor: Visit<Context>,
    Expression: Walk<Visitor, Context, Output = Visitor::Expression>,
{
    type Output = Visitor::Statement;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            super::Statement::Expression(x) => {
                let (x, v) = x.walk(v);

                v.statement(super::Statement::Expression(x), ctx)
            }

            super::Statement::Variable(binding, x) => {
                let (x, v) = x.walk(v);

                v.statement(super::Statement::Variable(binding, x), ctx)
            }
        }
    }
}
