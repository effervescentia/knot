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
    expression::statement::Statement,
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

fn fragment_to_binding(x: &Fragment) -> Option<String> {
    match x {
        Fragment::Statement(Statement::Variable(name, ..))
        | Fragment::Declaration(
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
        ) => Some(name.clone()),

        _ => None,
    }
}

fn unpack_module<'a, T>(x: ModuleNode<T, ()>) -> (ModuleNode<T, NodeContext>, RefCell<FileContext>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let file_ctx = FileContext::new();
    let cell = RefCell::new(file_ctx);
    let mut scope_ctx = ScopeContext::new(&cell);

    let result = x.register(&mut scope_ctx);

    let (refs, bindings) = scope_ctx.file.borrow().fragments.iter().fold(
        (vec![], vec![]),
        |(mut refs, mut bindings): (Vec<(usize, WeakRef)>, Vec<((Vec<usize>, String), usize)>),
         (id, (scope, x))| {
            refs.push((*id, x.weak()));

            if let Some(name) = fragment_to_binding(x) {
                bindings.push(((scope.clone(), name), *id));
            }

            (refs, bindings)
        },
    );

    scope_ctx.file.borrow_mut().weak_refs.extend(refs);
    scope_ctx.file.borrow_mut().bindings.extend(bindings);

    (result, cell)
}

pub fn analyze<T>(x: ModuleNode<T, ()>) -> ModuleNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let (result, _) = unpack_module(x);

    result
}

#[cfg(test)]
mod tests {
    use super::{fragment::Fragment, Type, WeakRef, WeakType};
    use crate::{
        parser::{
            declaration::{
                storage::{Storage, Visibility},
                Declaration,
            },
            expression::{primitive::Primitive, Expression},
            module::Module,
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::collections::HashMap;

    #[test]
    pub fn unpack_module() {
        let (_, cell) = super::unpack_module(f::mr(Module::new(
            vec![],
            vec![
                f::dc(
                    Declaration::TypeAlias {
                        name: Storage(Visibility::Public, String::from("MyType")),
                        value: f::txc(TypeExpression::Nil, ()),
                    },
                    (),
                ),
                f::dc(
                    Declaration::Enumerated {
                        name: Storage(Visibility::Public, String::from("MyEnum")),
                        variants: vec![],
                    },
                    (),
                ),
                f::dc(
                    Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("MY_CONST")),
                        value_type: None,
                        value: f::xc(Expression::Primitive(Primitive::Nil), ()),
                    },
                    (),
                ),
                f::dc(
                    Declaration::Function {
                        name: Storage(Visibility::Public, String::from("my_func")),
                        parameters: vec![],
                        body_type: None,
                        body: f::xc(Expression::Primitive(Primitive::Nil), ()),
                    },
                    (),
                ),
                f::dc(
                    Declaration::View {
                        name: Storage(Visibility::Public, String::from("MyView")),
                        parameters: vec![],
                        body: f::xc(Expression::Primitive(Primitive::Nil), ()),
                    },
                    (),
                ),
                f::dc(
                    Declaration::Module {
                        name: Storage(Visibility::Public, String::from("my_mod")),
                        value: f::mr(Module::new(vec![], vec![])),
                    },
                    (),
                ),
            ],
        )));

        assert_eq!(
            cell.borrow().fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Declaration(Declaration::TypeAlias {
                            name: Storage(Visibility::Public, String::from("MyType")),
                            value: 0,
                        })
                    )
                ),
                (
                    2,
                    (
                        vec![0],
                        Fragment::Declaration(Declaration::Enumerated {
                            name: Storage(Visibility::Public, String::from("MyEnum")),
                            variants: vec![],
                        })
                    )
                ),
                (
                    3,
                    (
                        vec![0, 3],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    4,
                    (
                        vec![0],
                        Fragment::Declaration(Declaration::Constant {
                            name: Storage(Visibility::Public, String::from("MY_CONST")),
                            value_type: None,
                            value: 3,
                        })
                    )
                ),
                (
                    5,
                    (
                        vec![0, 4],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    6,
                    (
                        vec![0],
                        Fragment::Declaration(Declaration::Function {
                            name: Storage(Visibility::Public, String::from("my_func")),
                            parameters: vec![],
                            body_type: None,
                            body: 5,
                        })
                    )
                ),
                (
                    7,
                    (
                        vec![0, 5],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    8,
                    (
                        vec![0],
                        Fragment::Declaration(Declaration::View {
                            name: Storage(Visibility::Public, String::from("MyView")),
                            parameters: vec![],
                            body: 7,
                        })
                    )
                ),
                (
                    9,
                    (vec![0, 6], Fragment::Module(Module::new(vec![], vec![])))
                ),
                (
                    10,
                    (
                        vec![0],
                        Fragment::Declaration(Declaration::Module {
                            name: Storage(Visibility::Public, String::from("my_mod")),
                            value: 9,
                        })
                    )
                ),
                (
                    11,
                    (
                        vec![0],
                        Fragment::Module(Module::new(vec![], vec![1, 2, 4, 6, 8, 10]))
                    )
                ),
            ])
        );

        assert_eq!(
            cell.borrow().weak_refs,
            HashMap::from_iter(vec![
                (0, WeakRef::Type(WeakType::Strong(Type::Nil))),
                (1, WeakRef::Type(WeakType::Reference(0))),
                (
                    2,
                    WeakRef::Value(WeakType::Strong(Type::Enumerated(vec![])))
                ),
                (3, WeakRef::Value(WeakType::Strong(Type::Nil))),
                (4, WeakRef::Value(WeakType::Reference(3))),
                (5, WeakRef::Value(WeakType::Strong(Type::Nil))),
                (6, WeakRef::Value(WeakType::Any)),
                (7, WeakRef::Value(WeakType::Strong(Type::Nil))),
                (8, WeakRef::Value(WeakType::Any)),
                (9, WeakRef::Value(WeakType::Any)),
                (10, WeakRef::Value(WeakType::Any)),
                (11, WeakRef::Value(WeakType::Any)),
            ])
        );

        assert_eq!(
            cell.borrow().bindings,
            HashMap::from_iter(vec![
                ((vec![0], String::from("MyType")), 1),
                ((vec![0], String::from("MyEnum")), 2),
                ((vec![0], String::from("MY_CONST")), 4),
                ((vec![0], String::from("my_func")), 6),
                ((vec![0], String::from("MyView")), 8),
                ((vec![0], String::from("my_mod")), 10),
            ])
        );
    }

    #[test]
    pub fn binding_type_alias() {
        assert_eq!(
            super::fragment_to_binding(&Fragment::Declaration(Declaration::TypeAlias {
                name: Storage(Visibility::Public, String::from("Foo")),
                value: 0
            })),
            Some(String::from("Foo"))
        );
    }

    #[test]
    pub fn binding_enumerated() {
        assert_eq!(
            super::fragment_to_binding(&Fragment::Declaration(Declaration::Enumerated {
                name: Storage(Visibility::Public, String::from("Foo")),
                variants: vec![]
            })),
            Some(String::from("Foo"))
        );
    }

    #[test]
    pub fn binding_constant() {
        assert_eq!(
            super::fragment_to_binding(&Fragment::Declaration(Declaration::Constant {
                name: Storage(Visibility::Public, String::from("FOO")),
                value_type: None,
                value: 0
            })),
            Some(String::from("FOO"))
        );
    }

    #[test]
    pub fn binding_function() {
        assert_eq!(
            super::fragment_to_binding(&Fragment::Declaration(Declaration::Function {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![],
                body_type: None,
                body: 0
            })),
            Some(String::from("foo"))
        );
    }

    #[test]
    pub fn binding_view() {
        assert_eq!(
            super::fragment_to_binding(&Fragment::Declaration(Declaration::View {
                name: Storage(Visibility::Public, String::from("Foo")),
                parameters: vec![],
                body: 0
            })),
            Some(String::from("Foo"))
        );
    }

    #[test]
    pub fn binding_module() {
        assert_eq!(
            super::fragment_to_binding(&Fragment::Declaration(Declaration::Module {
                name: Storage(Visibility::Public, String::from("foo")),
                value: 0
            })),
            Some(String::from("foo"))
        );
    }
}
