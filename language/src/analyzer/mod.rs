pub mod context;
mod declaration;
mod expression;
mod fragment;
mod ksx;
mod module;
mod statement;
mod type_expression;
use self::{
    context::{NodeContext, ScopeContext},
    fragment::Fragment,
};
use crate::parser::{
    declaration::{storage::Storage, Declaration},
    module::ModuleNode,
    position::Decrement,
};
use combine::Stream;
use context::FileContext;
use std::{cell::RefCell, fmt::Debug};

pub trait Analyze: Sized {
    type Ref;
    type Node;
    type Value<C>;

    fn register(self, ctx: &mut ScopeContext) -> Self::Node;

    fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<NodeContext>;

    fn to_ref<'a>(value: &'a Self::Value<NodeContext>) -> Self::Ref;
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

pub fn analyze<T>(x: ModuleNode<T, ()>) -> ModuleNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let file_ctx = FileContext::new();
    let cell = RefCell::new(file_ctx);
    let mut scope_ctx = ScopeContext::new(&cell);

    let result = x.register(&mut scope_ctx);

    scope_ctx
        .file
        .borrow()
        .fragments
        .iter()
        .for_each(|(id, (scope, x))| {
            scope_ctx.file.borrow_mut().weak_refs.insert(*id, x.weak());

            match x {
                Fragment::Declaration(
                    Declaration::TypeAlias {
                        name: Storage(_, name),
                        ..
                    }
                    | Declaration::Enumerated {
                        name: Storage(_, name),
                        ..
                    }
                    | Declaration::Constant {
                        name: Storage(_, name),
                        ..
                    }
                    | Declaration::Function {
                        name: Storage(_, name),
                        ..
                    }
                    | Declaration::View {
                        name: Storage(_, name),
                        ..
                    }
                    | Declaration::Module {
                        name: Storage(_, name),
                        ..
                    },
                ) => {
                    scope_ctx
                        .file
                        .borrow_mut()
                        .bindings
                        .insert((scope.clone(), name.clone()), *id);
                }
                _ => todo!(),
            }
        });

    result
}
