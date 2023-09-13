pub mod context;
mod declaration;
mod expression;
mod fragment;
mod ksx;
mod module;
mod type_expression;
use crate::parser::{module::ModuleNode, position::Decrement};
use combine::Stream;
use context::FileContext;
use std::{cell::RefCell, fmt::Debug};

use self::context::ScopeContext;

pub trait Analyze<Result, Ref>: Sized {
    type Value<C>;

    fn register(self, ctx: &mut ScopeContext) -> Result;

    fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<usize>;

    fn to_ref<'a>(value: &'a Self::Value<usize>) -> Ref;
}

#[derive(Debug, Clone, PartialEq)]
pub enum Type<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,
    Enumerated(Vec<(String, Vec<T>)>),
    Function(Vec<T>, T),
    View(Vec<T>, T),
    Module(Vec<(String, T)>),
}

#[derive(Debug, Clone, PartialEq)]
pub enum WeakType {
    Any,
    Number,
    Strong(Type<usize>),
    Reference(usize),
    NotFound(String),
}

#[derive(Debug, Clone, PartialEq)]
pub enum WeakRef {
    Type(WeakType),
    Value(WeakType),
}

pub fn analyze<T>(x: ModuleNode<T, ()>) -> ModuleNode<T, usize>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let file_ctx = FileContext::new();
    let cell = RefCell::new(file_ctx);
    let mut scope_ctx = ScopeContext::new(&cell);

    let result = x.register(&mut scope_ctx);

    let mut file_ctx = scope_ctx.file.borrow_mut();
    let iter = file_ctx
        .fragments
        .iter()
        .map(|(id, x)| (*id, x.weak()))
        .collect::<Vec<_>>();

    file_ctx.weak_refs.extend(iter);

    result
}
