use crate::Options;
use knot_language::ast::ModuleShape;

pub struct JavaScript(Vec<Statement>);

impl JavaScript {
    fn from_module(value: &ModuleShape, opts: &Options) -> Self {
        let statements = vec![
            Statement::from_module(value, opts),
            value
                .0
                .declarations
                .iter()
                .filter_map(|x| {
                    if x.0.is_public() {
                        Some(Statement::Export(x.0.name().clone()))
                    } else {
                        None
                    }
                })
                .collect(),
        ]
        .concat();

        Self(Statement::from_module(value, opts))
    }
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

#[derive(Clone, Debug, PartialEq)]
pub enum Statement {
    Expression(Expression),
    Variable(String, Expression),
    // Assignment(Expression, Expression),
    Return(Option<Expression>),
    // DefaultImport(String, String),
    // Import(String, Vec<(String, Option<String>)>),
    Export(String),
    // EmptyExport,
}

impl Statement {
    pub fn internal_variable(name: &str, x: Expression) -> Self {
        Self::Variable(name.to_string(), x)
    }
}
