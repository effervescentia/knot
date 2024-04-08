mod context;
mod error;
#[cfg(test)]
mod fixture;
mod infer;
mod into_typed;
mod semantic;
#[cfg(test)]
mod test;

pub use context::{Context, ModuleMap, TypeMap};
use error::Error;
use lang::{ast, NodeId};

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub enum AmbientScope {
    Element,
}

/// analysis result with possible resolution errors
pub type Result<Value> = std::result::Result<Value, Vec<(NodeId, Error)>>;

pub fn analyze<Raw>(ctx: &Context, raw: Raw) -> Result<(ast::typed::Program, TypeMap)>
where
    Raw: ast::into_fragments::IntoFragments<NodeId> + into_typed::IntoTyped + Clone,
{
    let fragments = raw.clone().into_fragments();

    let weak = infer::weak::infer_types(&fragments);
    let strong = infer::strong::infer_types(ctx, weak)?;

    let mut typed = raw.into_typed(ctx, &strong);
    let types = strong.canonicalize(ctx);

    typed = semantic::analyze(ctx, fragments, typed)?;

    Ok((typed, types))
}

#[cfg(test)]
mod tests {
    use crate::{analyze_mock, fixture, AmbientScope, ModuleMap};
    use kore::{assert_eq_sorted, str};
    use lang::{
        ast,
        types::{Kind, Type},
        CanonicalId, Namespace, NamespaceId, NamespaceKind, Node, NodeId,
    };
    use std::{collections::HashMap, rc::Rc};

    fn program(
        imports: Vec<ast::raw::Import>,
        declarations: Vec<ast::raw::Declaration>,
    ) -> ast::raw::Program {
        ast::meta::Program(ast::meta::Module::mock(ast::Module::new(
            imports,
            declarations,
        )))
    }

    #[test]
    fn empty_module() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let raw = program(vec![], vec![]);

        assert_eq_sorted!(
            super::analyze(&ctx, raw),
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![]),
                    (CanonicalId::mock(0), ast::typed::Type(Type::Module(vec![])))
                ))),
                HashMap::from_iter(vec![(
                    CanonicalId::mock(0),
                    Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Module(vec![]))))
                )])
            ))
        );
    }

    #[test]
    fn type_alias() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let raw = program(vec![], vec![fixture::type_alias::mock()]);

        assert_eq_sorted!(
            super::analyze(&ctx, raw),
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::type_alias::typed()]),
                    (
                        CanonicalId::mock(2),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MyTypeAlias"),
                            Kind::Type,
                            Rc::new((CanonicalId::mock(0), fixture::type_alias::type_of()))
                        )]))
                    )
                ))),
                fixture::type_alias::strong_types()
            ))
        );
    }

    #[test]
    fn constant() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let raw = program(vec![], vec![fixture::constant::mock()]);

        assert_eq_sorted!(
            super::analyze(&ctx, raw),
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::constant::typed()]),
                    (
                        CanonicalId::mock(3),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MY_CONSTANT"),
                            Kind::Value,
                            Rc::new((CanonicalId::mock(0), fixture::constant::type_of()))
                        )]))
                    )
                ))),
                fixture::constant::strong_types()
            ))
        );
    }

    #[test]
    fn enumerated() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let raw = program(vec![], vec![fixture::enumerated::mock()]);

        assert_eq_sorted!(
            super::analyze(&ctx, raw),
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::enumerated::typed()]),
                    (
                        CanonicalId::mock(3),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MyEnum"),
                            Kind::Mixed,
                            Rc::new((CanonicalId::mock(2), fixture::enumerated::type_of()))
                        )]))
                    )
                ))),
                fixture::enumerated::strong_types()
            ))
        );
    }

    #[ignore = "parameter inference not implemented"]
    #[test]
    fn function() {
        let mock = analyze_mock!();
        let ctx = mock.context();
        let raw = program(vec![], vec![fixture::function::mock()]);

        assert_eq_sorted!(
            super::analyze(&ctx, raw),
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::function::typed()]),
                    (
                        CanonicalId::mock(7),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MyEnum"),
                            Kind::Mixed,
                            Rc::new((CanonicalId::mock(0), fixture::function::type_of()))
                        )]))
                    )
                ))),
                HashMap::from_iter(vec![])
            ))
        );
    }

    #[test]
    fn view() {
        let mock = analyze_mock!(
            ambient = &HashMap::from_iter(vec![(AmbientScope::Element, NamespaceId(1))]),
            modules = &ModuleMap {
                keys: HashMap::new(),
                by_key: HashMap::from_iter(vec![(
                    NamespaceId(1),
                    (
                        CanonicalId(NamespaceId(1), NodeId(0)),
                        HashMap::from_iter(vec![
                            (str!("div"), CanonicalId(NamespaceId(1), NodeId(1))),
                            (str!("h1"), CanonicalId(NamespaceId(1), NodeId(2))),
                            (str!("main"), CanonicalId(NamespaceId(1), NodeId(3))),
                        ]),
                        HashMap::from_iter(vec![
                            (
                                CanonicalId(NamespaceId(1), NodeId(1)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(1)),
                                    ast::typed::Type(Type::View(vec![])),
                                )),
                            ),
                            (
                                CanonicalId(NamespaceId(1), NodeId(2)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(2)),
                                    ast::typed::Type(Type::View(vec![])),
                                )),
                            ),
                            (
                                CanonicalId(NamespaceId(1), NodeId(3)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(3)),
                                    ast::typed::Type(Type::View(vec![])),
                                )),
                            ),
                        ]),
                    ),
                )]),
            }
        );
        let ctx = mock.context();
        let raw = program(vec![], vec![fixture::view::mock()]);

        assert_eq_sorted!(
            super::analyze(&ctx, raw),
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::view::typed()]),
                    (
                        CanonicalId::mock(21),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MyView"),
                            Kind::Value,
                            Rc::new((CanonicalId::mock(20), fixture::view::type_of()))
                        )]))
                    )
                ))),
                fixture::view::strong_types()
            ))
        );
    }

    #[test]
    fn module() {
        let mock = analyze_mock!(
            modules = &ModuleMap {
                keys: HashMap::from_iter(vec![(
                    Namespace(NamespaceKind::Internal, vec![str!("theme")]),
                    NamespaceId(1),
                )]),
                by_key: HashMap::from_iter(vec![(
                    NamespaceId(1),
                    (
                        CanonicalId(NamespaceId(1), NodeId(0)),
                        HashMap::new(),
                        HashMap::from_iter(vec![
                            (
                                CanonicalId(NamespaceId(1), NodeId(0)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(0)),
                                    ast::typed::Type(Type::Module(vec![(
                                        str!("PRIMARY"),
                                        Kind::Value,
                                        Rc::new((
                                            CanonicalId(NamespaceId(1), NodeId(1)),
                                            ast::typed::Type(Type::String),
                                        )),
                                    )])),
                                )),
                            ),
                            (
                                CanonicalId(NamespaceId(1), NodeId(1)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(1)),
                                    ast::typed::Type(Type::String),
                                )),
                            ),
                        ]),
                    ),
                )]),
            }
        );
        let ctx = mock.context();
        let raw = program(vec![], vec![fixture::module::mock()]);

        assert_eq_sorted!(
            super::analyze(&ctx, raw),
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::module::typed()]),
                    (
                        CanonicalId::mock(8),
                        ast::typed::Type(Type::Module(vec![(
                            str!("my_module"),
                            Kind::Mixed,
                            Rc::new((CanonicalId::mock(6), fixture::module::type_of()))
                        )]))
                    )
                ))),
                fixture::module::strong_types()
            ))
        );
    }
}
