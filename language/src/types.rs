use std::fmt::Debug;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum RefKind {
    Type,
    Value,
    Mixed,
}

#[derive(Clone, Debug, PartialEq)]
pub enum EnumeratedType<T> {
    Declaration(Vec<(String, Vec<T>)>),
    Variant(Vec<T>, T),
    Instance(T),
}

#[derive(Clone, Debug, PartialEq)]
pub enum Type<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,

    Enumerated(EnumeratedType<T>),
    Function(Vec<T>, T),
    View(Vec<T>),
    Module(Vec<(String, RefKind, T)>),
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum TypePrimitive {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,
}

#[derive(Clone, Debug, PartialEq)]
pub enum TypeExpression<TExpr> {
    Primitive(TypePrimitive),
    Identifier(String),
    Group(Box<TExpr>),
    DotAccess(Box<TExpr>, String),
    Function(Vec<TExpr>, Box<TExpr>),
    // View(Vec<(String, TypeExpression)>),
}
