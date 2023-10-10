use super::statement::Statement;
use crate::Options;
use knot_language::{
    ast::{self, ExpressionShape, KSXShape, Primitive},
    formatter::{Block, Indented, SeparateEach, TerminateEach},
};
use std::fmt::{Display, Formatter};

/// [JavaScript escape characters](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Lexical_grammar#string_literals)
fn escape_string(s: &String) -> String {
    s.replace("\\", "\\\\")
        .replace("\"", "\\\"")
        .replace("\n", "\\n")
        .replace("\t", "\\t")
        .replace("\r", "\\r")
        .replace('\u{007F}', "\\b")
        .replace('\u{000C}', "\\f")
        .replace('\u{000B}', "\\v")
}

#[derive(Clone, Debug, PartialEq)]
pub enum Expression {
    Null,
    Boolean(bool),
    Number(String),
    String(String),
    Group(Box<Expression>),
    Identifier(String),
    UnaryOperation(&'static str, Box<Expression>),
    BinaryOperation(&'static str, Box<Expression>, Box<Expression>),
    // Ternary(Box<Expression>, Box<Expression>, Box<Expression>),
    DotAccess(Box<Expression>, String),
    FunctionCall(Box<Expression>, Vec<Expression>),
    Function(Option<String>, Vec<String>, Vec<Statement>),
    Array(Vec<Expression>),
    Object(Vec<(String, Expression)>),

    // JavaScript has no explicit closure syntax
    // we are using an immediately invoked function to achieve this instead
    Closure(Vec<Statement>),
}

impl Expression {
    pub fn global(name: &str) -> Self {
        Self::Identifier(name.to_string())
    }

    pub fn call_global(name: &str, arguments: Vec<Expression>) -> Self {
        Self::FunctionCall(Box::new(Self::global(name)), arguments)
    }

    pub fn plugin(namespace: &str, feature: &str) -> Self {
        Self::call_global(
            "$knot.plugin.get",
            vec![
                Expression::String(namespace.to_string()),
                Expression::String(feature.to_string()),
                Expression::String(String::from("1.0")),
            ],
        )
    }
}

impl Expression {
    pub fn from_expression(value: &ExpressionShape, opts: &Options) -> Self {
        match &value.0 {
            ast::Expression::Primitive(x) => match x {
                Primitive::Nil => Self::Null,

                Primitive::Boolean(x) => Self::Boolean(*x),

                Primitive::Integer(x) => Self::Number(format!("{x}")),

                Primitive::Float(x, precision) => {
                    Self::Number(format!("{x:.0$}", *precision as usize))
                }

                Primitive::String(x) => {
                    Self::Number(format!("{escaped}", escaped = escape_string(&x)))
                }
            },

            ast::Expression::Identifier(x) => Self::Identifier(x.clone()),

            ast::Expression::Group(x) => Self::Group(Box::new(Self::from_expression(&x, opts))),

            ast::Expression::Closure(xs) if xs.is_empty() => Self::Null,
            ast::Expression::Closure(xs) => {
                let statements = xs
                    .iter()
                    .enumerate()
                    .map(|(index, x)| {
                        if index == xs.len() - 1 {
                            Statement::from_last_statement(x, opts)
                        } else {
                            Statement::from_statement(x, opts)
                        }
                    })
                    .collect::<Vec<_>>();

                Expression::Closure(statements)
            }

            ast::Expression::UnaryOperation(op, x) => match op {
                ast::UnaryOperator::Not => {
                    Self::UnaryOperation("!", Box::new(Self::from_expression(&x, opts)))
                }

                ast::UnaryOperator::Negate => {
                    Self::UnaryOperation("-", Box::new(Self::from_expression(&x, opts)))
                }

                ast::UnaryOperator::Absolute => {
                    Self::call_global("Math.abs", vec![Self::from_expression(&x, opts)])
                }
            },

            ast::Expression::BinaryOperation(op, lhs, rhs) => {
                let binary_op = |op| {
                    Self::BinaryOperation(
                        op,
                        Box::new(Self::from_expression(&lhs, opts)),
                        Box::new(Self::from_expression(&rhs, opts)),
                    )
                };

                match op {
                    ast::BinaryOperator::And => binary_op("&&"),
                    ast::BinaryOperator::Or => binary_op("||"),

                    ast::BinaryOperator::Equal => binary_op("==="),
                    ast::BinaryOperator::NotEqual => binary_op("!=="),

                    ast::BinaryOperator::LessThan => binary_op("<"),
                    ast::BinaryOperator::LessThanOrEqual => binary_op("<="),
                    ast::BinaryOperator::GreaterThan => binary_op(">"),
                    ast::BinaryOperator::GreaterThanOrEqual => binary_op(">="),

                    ast::BinaryOperator::Add => binary_op("+"),
                    ast::BinaryOperator::Subtract => binary_op("-"),
                    ast::BinaryOperator::Multiply => binary_op("*"),
                    ast::BinaryOperator::Divide => binary_op("/"),

                    ast::BinaryOperator::Exponent => Self::call_global(
                        "Math.pow",
                        vec![
                            Self::from_expression(&lhs, opts),
                            Self::from_expression(&rhs, opts),
                        ],
                    ),
                }
            }

            ast::Expression::DotAccess(lhs, rhs) => {
                Self::DotAccess(Box::new(Self::from_expression(&lhs, opts)), rhs.clone())
            }

            ast::Expression::FunctionCall(x, arguments) => Self::FunctionCall(
                Box::new(Self::from_expression(&x, opts)),
                arguments
                    .iter()
                    .map(|x| Self::from_expression(x, opts))
                    .collect(),
            ),

            ast::Expression::Style(xs) => Self::FunctionCall(
                Box::new(Self::plugin("style", "create")),
                vec![Self::Object(
                    xs.iter()
                        .map(|(key, value)| (key.clone(), Self::from_expression(value, opts)))
                        .collect(),
                )],
            ),

            ast::Expression::KSX(x) => Self::from_ksx(&x, opts),
        }
    }

    pub fn from_ksx(value: &KSXShape, opts: &Options) -> Self {
        match &value.0 {
            ast::KSX::Text(x) => Expression::String(x.clone()),

            ast::KSX::Inline(x) => Expression::from_expression(&x, opts),

            ast::KSX::Fragment(xs) => Expression::FunctionCall(
                Box::new(Self::plugin("ksx", "createFragment")),
                xs.iter().map(|x| Self::from_ksx(x, opts)).collect(),
            ),

            ast::KSX::ClosedElement(tag, attributes) => Self::FunctionCall(
                Box::new(Self::plugin("ksx", "createElement")),
                vec![
                    Expression::Identifier(tag.clone()),
                    Expression::from_attributes(attributes, opts),
                ],
            ),

            ast::KSX::OpenElement(start_tag, attributes, children, ..) => {
                let name_arg = Expression::Identifier(start_tag.clone());

                Self::FunctionCall(
                    Box::new(Self::plugin("ksx", "createElement")),
                    if attributes.is_empty() && children.is_empty() {
                        vec![name_arg]
                    } else {
                        vec![
                            vec![name_arg, Expression::from_attributes(attributes, opts)],
                            children.iter().map(|x| Self::from_ksx(x, opts)).collect(),
                        ]
                        .concat()
                    },
                )
            }
        }
    }

    pub fn from_attributes(
        xs: &Vec<(String, Option<ast::ExpressionShape>)>,
        opts: &Options,
    ) -> Self {
        if !xs.is_empty() {
            return Self::Null;
        }

        Self::Object(
            xs.iter()
                .map(|(name, x)| {
                    (
                        name.clone(),
                        match x {
                            Some(x) => Expression::from_expression(x, opts),
                            None => Expression::Identifier(name.clone()),
                        },
                    )
                })
                .collect(),
        )
    }
}

impl Display for Expression {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Null => write!(f, "null"),

            Self::Boolean(true) => write!(f, "true"),
            Self::Boolean(false) => write!(f, "false"),

            Self::Number(x) => write!(f, "{x}"),

            Self::String(x) => write!(f, "\"{encoded}\"", encoded = escape_string(x)),

            Self::Identifier(x) => write!(f, "{x}"),

            Self::Group(x) => write!(f, "({x})"),

            Self::UnaryOperation(op, x) => write!(f, "{op}{x}"),

            Self::BinaryOperation(op, lhs, rhs) => write!(f, "{lhs} {op} {rhs}"),

            Self::DotAccess(lhs, rhs) => write!(f, "{lhs}.{rhs}"),

            Self::FunctionCall(x, arguments) => {
                write!(
                    f,
                    "{x}({arguments})",
                    arguments = SeparateEach(", ", arguments)
                )
            }

            Self::Array(xs) => {
                write!(f, "[{items}]", items = SeparateEach(", ", xs))
            }

            Self::Object(xs) => {
                write!(
                    f,
                    "{{{properties}}}",
                    properties = SeparateEach(
                        ", ",
                        &xs.iter().map(|(key, value)| Property(key, value)).collect()
                    )
                )
            }

            Self::Function(name, parameters, statements) => {
                write!(
                    f,
                    "function{name}({parameters}) {{{statements}}}",
                    name = name.as_ref().map(|x| format!(" {x}")).unwrap_or_default(),
                    parameters = SeparateEach(",", parameters),
                    statements = Indented(Block(TerminateEach("\n", statements))),
                )
            }

            Self::Closure(xs) => {
                write!(
                    f,
                    "{iife}",
                    iife = Self::FunctionCall(
                        Box::new(Self::Group(Box::new(Self::Function(
                            None,
                            vec![],
                            xs.clone()
                        )))),
                        vec![],
                    )
                )
            }
        }
    }
}

struct Property<'a>(&'a String, &'a Expression);

impl<'a> Display for Property<'a> {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(f, "{key}: {value}", key = self.0, value = self.1)
    }
}
