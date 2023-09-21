pub mod context;
mod declaration;
mod expression;
pub mod fragment;
mod infer;
mod ksx;
mod module;
mod parameter;
mod register;
mod statement;
mod type_expression;

use crate::{ast::module::ModuleNode, common::position::Decrement};
use combine::Stream;
use context::{AnalyzeContext, FileContext, NodeContext, ScopeContext};
use infer::strong::ToStrong;
use register::Register;
use std::{cell::RefCell, fmt::Debug};

#[derive(Clone, Debug, PartialEq)]
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

#[derive(Clone, Debug, PartialEq)]
pub enum Weak {
    Unknown,
    Type(Type<usize>),
    Inherit(usize),
}

#[derive(Clone, Debug, PartialEq)]
pub enum Strong {
    NotFound(String),
    Type(Type<usize>),
}

#[derive(Clone, Debug, PartialEq)]
pub enum RefKind {
    Type,
    Value,
}

pub type WeakRef = (RefKind, Weak);
pub type StrongRef = (RefKind, Strong);

fn register_fragments<T>(
    x: ModuleNode<T, ()>,
    file_ctx: &RefCell<FileContext>,
) -> ModuleNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    x.register(&mut ScopeContext::new(file_ctx))
}

pub fn analyze<T>(x: ModuleNode<T, ()>) -> ModuleNode<T, Strong>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    // register AST fragments depth-first with monotonically increasing IDs
    let file_ctx = RefCell::new(FileContext::new());
    let untyped = register_fragments(x, &file_ctx);

    // apply weak type inference
    let mut analyze_ctx = AnalyzeContext::new(&file_ctx);
    infer::weak::infer_types(&mut analyze_ctx);

    // apply strong type inference
    infer::strong::infer_types(&mut analyze_ctx);

    if analyze_ctx.weak_refs.len() != analyze_ctx.strong_refs.len() {
        panic!("analysis failed to determine all types")
    }

    untyped.to_strong(&analyze_ctx)
}

#[cfg(test)]
mod tests {
    // TODO
}
