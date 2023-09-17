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
use self::context::{AnalyzeContext, NodeContext, ScopeContext};
use crate::parser::{module::ModuleNode, position::Decrement};
use combine::Stream;
use context::FileContext;
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
pub enum WeakType {
    Any,
    Strong(Type<usize>),
    Reference(usize),
}

#[derive(Clone, Debug, PartialEq)]
pub enum RefKind {
    Type,
    Value,
}

pub type WeakRef = (RefKind, WeakType);
pub type StrongRef = (RefKind, Type<usize>);

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

pub fn analyze<T>(x: ModuleNode<T, ()>) -> ModuleNode<T, NodeContext>
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

    untyped
}

#[cfg(test)]
mod tests {
    use super::fragment::Fragment;
    use crate::{
        analyzer::context::FileContext,
        parser::{
            expression::{primitive::Primitive, Expression},
            module::Module,
            statement::Statement,
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::{cell::RefCell, collections::BTreeMap};

    #[test]
    fn register_declaration_fragments() {
        let file_ctx = RefCell::new(FileContext::new());

        super::register_fragments(
            f::n::mr(Module::new(
                vec![],
                vec![
                    f::n::d(f::a::type_("MyType", f::n::tx(TypeExpression::Nil))),
                    f::n::d(f::a::const_(
                        "MY_CONST",
                        None,
                        f::n::x(Expression::Primitive(Primitive::Nil)),
                    )),
                ],
            )),
            &file_ctx,
        );

        assert_eq!(
            file_ctx.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    1,
                    (vec![0], Fragment::Declaration(f::a::type_("MyType", 0)))
                ),
                (
                    2,
                    (
                        vec![0, 2],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    3,
                    (
                        vec![0],
                        Fragment::Declaration(f::a::const_("MY_CONST", None, 2))
                    )
                ),
                (
                    4,
                    (vec![0], Fragment::Module(Module::new(vec![], vec![1, 3])))
                ),
            ])
        );
    }

    #[test]
    fn register_closure_fragments() {
        let file_ctx = RefCell::new(FileContext::new());

        super::register_fragments(
            f::n::mr(Module::new(
                vec![],
                vec![
                    f::n::d(f::a::const_(
                        "FOO",
                        None,
                        f::n::x(Expression::Primitive(Primitive::Nil)),
                    )),
                    f::n::d(f::a::const_(
                        "BAR",
                        None,
                        f::n::x(Expression::Closure(vec![
                            f::n::s(Statement::Variable(
                                String::from("foo"),
                                f::n::x(Expression::Identifier(String::from("FOO"))),
                            )),
                            f::n::s(Statement::Effect(f::n::x(Expression::Identifier(
                                String::from("foo"),
                            )))),
                        ])),
                    )),
                ],
            )),
            &file_ctx,
        );

        assert_eq!(
            file_ctx.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (vec![0], Fragment::Declaration(f::a::const_("FOO", None, 0)))
                ),
                (
                    2,
                    (
                        vec![0, 2, 3],
                        Fragment::Expression(Expression::Identifier(String::from("FOO")))
                    )
                ),
                (
                    3,
                    (
                        vec![0, 2, 3],
                        Fragment::Statement(Statement::Variable(String::from("foo"), 2))
                    )
                ),
                (
                    4,
                    (
                        vec![0, 2, 3],
                        Fragment::Expression(Expression::Identifier(String::from("foo")))
                    )
                ),
                (
                    5,
                    (vec![0, 2, 3], Fragment::Statement(Statement::Effect(4)))
                ),
                (
                    6,
                    (
                        vec![0, 2],
                        Fragment::Expression(Expression::Closure(vec![3, 5]))
                    )
                ),
                (
                    7,
                    (vec![0], Fragment::Declaration(f::a::const_("BAR", None, 6)))
                ),
                (
                    8,
                    (vec![0], Fragment::Module(Module::new(vec![], vec![1, 7])))
                ),
            ])
        );
    }
}
