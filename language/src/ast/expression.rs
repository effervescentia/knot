use super::{BinaryOperator, UnaryOperator};
use crate::walk::{ProgramVisitor, Walk};
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

impl<Visitor, Context, Expression_, Statement, Component> Walk<Visitor>
    for (Expression<Expression_, Statement, Component>, Context)
where
    Visitor: ProgramVisitor<Context = Context>,
    Expression_: Walk<Visitor, Output = Visitor::Expression>,
    Statement: Walk<Visitor, Output = Visitor::Statement>,
    Component: Walk<Visitor, Output = Visitor::Component>,
{
    type Output = Visitor::Expression;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            Expression::Primitive(x) => v.expression(Expression::Primitive(x), ctx),

            Expression::Identifier(x) => v.expression(Expression::Identifier(x), ctx),

            Expression::Group(x) => {
                let (x, v) = x.walk(v);

                v.expression(Expression::Group(Box::new(x)), ctx)
            }

            Expression::Closure(xs) => {
                let (xs, v) = v.scoped(|v| xs.walk(v));

                v.expression(Expression::Closure(xs), ctx)
            }

            Expression::UnaryOperation(op, x) => {
                let (x, v) = x.walk(v);

                v.expression(Expression::UnaryOperation(op, Box::new(x)), ctx)
            }

            Expression::BinaryOperation(op, l, r) => {
                let (l, v) = l.walk(v);
                let (r, v) = r.walk(v);

                v.expression(
                    Expression::BinaryOperation(op, Box::new(l), Box::new(r)),
                    ctx,
                )
            }

            Expression::PropertyAccess(x, property) => {
                let (x, v) = x.walk(v);

                v.expression(Expression::PropertyAccess(Box::new(x), property), ctx)
            }

            Expression::FunctionCall(x, arguments) => {
                let (x, v) = x.walk(v);
                let (arguments, v) = arguments.walk(v);

                v.expression(Expression::FunctionCall(Box::new(x), arguments), ctx)
            }

            Expression::Component(x) => {
                let (x, v) = x.walk(v);

                v.expression(Expression::Component(Box::new(x)), ctx)
            }

            Expression::Style(xs) => {
                let (xs, v) = xs.walk(v);

                v.expression(Expression::Style(xs), ctx)
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

impl<Visitor, Context, Expression> Walk<Visitor> for (Statement<Expression>, Context)
where
    Visitor: ProgramVisitor<Context = Context>,
    Expression: Walk<Visitor, Output = Visitor::Expression>,
{
    type Output = Visitor::Statement;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            Statement::Expression(x) => {
                let (x, v) = x.walk(v);

                v.statement(Statement::Expression(x), ctx)
            }

            Statement::Variable(binding, x) => {
                let (x, v) = x.walk(v);

                v.statement(Statement::Variable(binding, x), ctx)
            }
        }
    }
}
