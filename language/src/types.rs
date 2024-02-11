use std::fmt::Debug;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum RefKind {
    Type,
    Value,
    Mixed,
}

#[derive(Clone, Debug, PartialEq)]
pub enum Enumerated<T> {
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

    Enumerated(Enumerated<T>),
    Function(Vec<T>, T),
    View(Vec<T>),
    Module(Vec<(String, RefKind, T)>),
}
