mod ast;
mod context;
// mod declaration;
// mod expression;
// mod fragment;
// mod component;
// mod import;
mod infer;
// mod module;
// mod parameter;
mod weak;
// mod register;
// mod statement;
#[cfg(test)]
mod test;
// mod type_expression;
// mod types;

use context::{NodeDescriptor, StrongContext, WeakContext};
pub use infer::strong::Strong;
use lang::{
    ast::{explode, walk},
    ModuleReference,
};
use std::collections::HashMap;
// use infer::strong::{SemanticError, ToStrong};
// use lang::ModuleReference;
// use register::Register;
// use std::{cell::RefCell, collections::HashMap, fmt::Debug};
// use types::Type;

type ModuleTypeMap = HashMap<ModuleReference, Strong>;

// #[derive(Clone, Debug, PartialEq)]
// pub struct FinalType(Result<Type<Box<FinalType>>, SemanticError>);

// #[derive(Clone, Debug, PartialEq)]
// pub struct PreviewType(Type<Box<PreviewType>>);

// #[derive(Clone, Copy, Debug, PartialEq)]
// pub enum RefKind {
//     Type,
//     Value,
//     Mixed,
// }

// fn register_fragments<R>(x: &Program<R, ()>) -> (Program<R, NodeContext>, FileContext)
// where
//     R: Copy,
// {
//     let file_ctx = RefCell::new(FileContext::new());
//     let untyped = x.0.register(&ScopeContext::new(&file_ctx));

//     (Program(untyped), file_ctx.into_inner())
// }

pub fn analyze<'a, Raw, R>(
    module_reference: &ModuleReference,
    raw: Raw,
    modules: &ModuleTypeMap,
) -> ast::typed::Program<'a>
where
    Raw: explode::Explode,
    R: Copy,
{
    // register AST fragments depth-first with monotonically increasing IDs
    let fragments = raw.explode();

    // apply weak type inference
    let weak = infer::weak::infer_types(fragments);

    // apply strong type inference
    let strong = infer::strong::infer_types(module_reference, modules, weak);

    raw.to_strong(&strong)
}

// impl<R> ToStrong<Program<R, Strong>> for Program<R, NodeContext>
// where
//     R: Copy,
// {
//     fn to_strong(&self, ctx: &StrongContext) -> Program<R, Strong> {
//         Program(self.0.to_strong(ctx))
//     }
// }

// #[cfg(test)]
// mod tests {
//     use crate::{test::fixture as f, types::Type, RefKind};
//     use kore::str;
//     use lang::{
//         ast::{Expression, Module, ModuleNode, Primitive, TypeExpression},
//         ModuleReference, ModuleScope, Program,
//     };
//     use std::collections::HashMap;

//     fn mock_reference(name: &str) -> ModuleReference {
//         ModuleReference(ModuleScope::Source, vec![name.to_owned()])
//     }

//     #[test]
//     fn empty_module() {
//         let ast = Program(f::n::m(Module::new(vec![], vec![])));
//         let modules = HashMap::from_iter(vec![]);

//         assert_eq!(
//             super::analyze(&mock_reference("foo"), &ast, &modules),
//             Program(ModuleNode(
//                 Module::new(vec![], vec![]),
//                 Ok(Type::Module(vec![]))
//             ))
//         );
//     }

//     mod import {
//         use super::*;
//         use lang::ast::{Import, ImportSource};

//         #[test]
//         fn module() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![f::n::i(Import {
//                     source: ImportSource::Local,
//                     path: vec![str!("foo")],
//                     alias: None,
//                 })],
//                 vec![],
//             )));
//             let modules = HashMap::from_iter(vec![(
//                 ModuleReference(ModuleScope::Source, vec![str!("foo")]),
//                 Ok(Type::Module(vec![(str!("bar"), RefKind::Value, 1)])),
//             )]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![f::n::ic(
//                             Import {
//                                 source: ImportSource::Local,
//                                 path: vec![str!("foo")],
//                                 alias: None
//                             },
//                             Ok(Type::Module(vec![(str!("bar"), RefKind::Value, 1)]))
//                         )],
//                         vec![]
//                     ),
//                     Ok(Type::Module(vec![]))
//                 ))
//             );
//         }

//         #[test]
//         fn aliased_module() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![f::n::i(Import {
//                     source: ImportSource::Local,
//                     path: vec![str!("foo")],
//                     alias: Some(str!("bar")),
//                 })],
//                 vec![],
//             )));
//             let modules = HashMap::from_iter(vec![(
//                 ModuleReference(ModuleScope::Source, vec![str!("foo")]),
//                 Ok(Type::Module(vec![(str!("bar"), RefKind::Value, 1)])),
//             )]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![f::n::ic(
//                             Import {
//                                 source: ImportSource::Local,
//                                 path: vec![str!("foo")],
//                                 alias: Some(str!("bar"))
//                             },
//                             Ok(Type::Module(vec![(str!("bar"), RefKind::Value, 1)])),
//                         )],
//                         vec![]
//                     ),
//                     Ok(Type::Module(vec![]))
//                 ))
//             );
//         }
//     }

//     mod type_alias {
//         use super::*;

//         #[test]
//         fn primitive() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::const_(
//                     "foo",
//                     None,
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::const_(
//                                 "foo",
//                                 None,
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::Nil)
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 1)]))
//                 ))
//             );
//         }
//     }

//     mod enumerated {
//         use super::*;

//         #[test]
//         fn static_variant() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::enum_("foo", vec![(str!("Bar"), vec![])]))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::enum_("foo", vec![(str!("Bar"), vec![])]),
//                             Ok(Type::Enumerated(vec![(str!("Bar"), vec![])]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Mixed, 0)]))
//                 ))
//             );
//         }

//         #[test]
//         fn parameterized_variant() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::enum_(
//                     "foo",
//                     vec![(
//                         str!("Bar"),
//                         vec![
//                             f::n::tx(TypeExpression::Nil),
//                             f::n::tx(TypeExpression::Boolean),
//                         ],
//                     )],
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::enum_(
//                                 "foo",
//                                 vec![(
//                                     str!("Bar"),
//                                     vec![
//                                         f::n::txc(TypeExpression::Nil, Ok(Type::Nil)),
//                                         f::n::txc(TypeExpression::Boolean, Ok(Type::Boolean))
//                                     ]
//                                 )]
//                             ),
//                             Ok(Type::Enumerated(vec![(str!("Bar"), vec![0, 1])]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Mixed, 2)]))
//                 ))
//             );
//         }
//     }

//     mod constant {
//         use super::*;

//         #[test]
//         fn without_typedef() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::const_(
//                     "foo",
//                     None,
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::const_(
//                                 "foo",
//                                 None,
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::Nil)
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 1)]))
//                 ))
//             );
//         }

//         #[test]
//         fn with_typedef() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::const_(
//                     "foo",
//                     Some(f::n::tx(TypeExpression::Nil)),
//                     f::n::x(Expression::Primitive(Primitive::Boolean(true))),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::const_(
//                                 "foo",
//                                 Some(f::n::txc(TypeExpression::Nil, Ok(Type::Nil))),
//                                 f::n::xc(
//                                     Expression::Primitive(Primitive::Boolean(true)),
//                                     Ok(Type::Boolean)
//                                 )
//                             ),
//                             Ok(Type::Nil)
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 2)]))
//                 ))
//             );
//         }
//     }

//     mod function {
//         use super::*;
//         use lang::ast::{BinaryOperator, Parameter};

//         #[test]
//         fn no_parameters() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::func_(
//                     "foo",
//                     vec![],
//                     None,
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::func_(
//                                 "foo",
//                                 vec![],
//                                 None,
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::Function(vec![], 0))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 1)]))
//                 ))
//             );
//         }

//         #[test]
//         fn parameters_with_defaults() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::func_(
//                     "foo",
//                     vec![
//                         f::n::p(Parameter {
//                             name: str!("bar"),
//                             value_type: None,
//                             default_value: Some(f::n::x(Expression::Primitive(
//                                 Primitive::Boolean(true),
//                             ))),
//                         }),
//                         f::n::p(Parameter {
//                             name: str!("fizz"),
//                             value_type: None,
//                             default_value: Some(f::n::x(Expression::Primitive(
//                                 Primitive::Integer(123),
//                             ))),
//                         }),
//                     ],
//                     None,
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::func_(
//                                 "foo",
//                                 vec![
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("bar"),
//                                             value_type: None,
//                                             default_value: Some(f::n::xc(
//                                                 Expression::Primitive(Primitive::Boolean(true),),
//                                                 Ok(Type::Boolean)
//                                             )),
//                                         },
//                                         Ok(Type::Boolean)
//                                     ),
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("fizz"),
//                                             value_type: None,
//                                             default_value: Some(f::n::xc(
//                                                 Expression::Primitive(Primitive::Integer(123)),
//                                                 Ok(Type::Integer)
//                                             )),
//                                         },
//                                         Ok(Type::Integer)
//                                     ),
//                                 ],
//                                 None,
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::Function(vec![1, 3], 4))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 5)]))
//                 ))
//             );
//         }

//         #[test]
//         fn parameters_with_typedefs() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::func_(
//                     "foo",
//                     vec![
//                         f::n::p(Parameter {
//                             name: str!("bar"),
//                             value_type: Some(f::n::tx(TypeExpression::Boolean)),
//                             default_value: None,
//                         }),
//                         f::n::p(Parameter {
//                             name: str!("fizz"),
//                             value_type: Some(f::n::tx(TypeExpression::Integer)),
//                             default_value: None,
//                         }),
//                     ],
//                     None,
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::func_(
//                                 "foo",
//                                 vec![
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("bar"),
//                                             value_type: Some(f::n::txc(
//                                                 TypeExpression::Boolean,
//                                                 Ok(Type::Boolean)
//                                             )),
//                                             default_value: None,
//                                         },
//                                         Ok(Type::Boolean)
//                                     ),
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("fizz"),
//                                             value_type: Some(f::n::txc(
//                                                 TypeExpression::Integer,
//                                                 Ok(Type::Integer)
//                                             )),
//                                             default_value: None,
//                                         },
//                                         Ok(Type::Integer)
//                                     ),
//                                 ],
//                                 None,
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::Function(vec![1, 3], 4))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 5)]))
//                 ))
//             );
//         }

//         #[test]
//         fn parameter_with_default_and_typedef() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::func_(
//                     "foo",
//                     vec![f::n::p(Parameter {
//                         name: str!("bar"),
//                         value_type: Some(f::n::tx(TypeExpression::Boolean)),
//                         default_value: Some(f::n::x(Expression::Primitive(Primitive::Integer(
//                             123,
//                         )))),
//                     })],
//                     None,
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::func_(
//                                 "foo",
//                                 vec![f::n::pc(
//                                     Parameter {
//                                         name: str!("bar"),
//                                         value_type: Some(f::n::txc(
//                                             TypeExpression::Boolean,
//                                             Ok(Type::Boolean)
//                                         )),
//                                         default_value: Some(f::n::xc(
//                                             Expression::Primitive(Primitive::Integer(123)),
//                                             Ok(Type::Integer)
//                                         )),
//                                     },
//                                     Ok(Type::Boolean)
//                                 )],
//                                 None,
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::Function(vec![2], 3))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 4)]))
//                 ))
//             );
//         }

//         #[test]
//         fn infer_return_type_from_parameter() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::func_(
//                     "foo",
//                     vec![f::n::p(Parameter {
//                         name: str!("bar"),
//                         value_type: Some(f::n::tx(TypeExpression::Boolean)),
//                         default_value: None,
//                     })],
//                     None,
//                     f::n::x(Expression::Identifier(str!("bar"))),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::func_(
//                                 "foo",
//                                 vec![f::n::pc(
//                                     Parameter {
//                                         name: str!("bar"),
//                                         value_type: Some(f::n::txc(
//                                             TypeExpression::Boolean,
//                                             Ok(Type::Boolean)
//                                         )),
//                                         default_value: None,
//                                     },
//                                     Ok(Type::Boolean)
//                                 )],
//                                 None,
//                                 f::n::xc(Expression::Identifier(str!("bar")), Ok(Type::Boolean))
//                             ),
//                             Ok(Type::Function(vec![1], 2))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 3)]))
//                 ))
//             );
//         }

//         #[test]
//         fn with_return_type() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::func_(
//                     "foo",
//                     vec![],
//                     Some(f::n::tx(TypeExpression::Boolean)),
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::func_(
//                                 "foo",
//                                 vec![],
//                                 Some(f::n::txc(TypeExpression::Boolean, Ok(Type::Boolean))),
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil)),
//                             ),
//                             Ok(Type::Function(vec![], 0))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 2)]))
//                 ))
//             );
//         }

//         #[test]
//         #[ignore = "todo"]
//         fn infer_parameters() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::func_(
//                     "foo",
//                     vec![
//                         f::n::p(Parameter {
//                             name: str!("bar"),
//                             value_type: None,
//                             default_value: None,
//                         }),
//                         f::n::p(Parameter {
//                             name: str!("fizz"),
//                             value_type: None,
//                             default_value: None,
//                         }),
//                     ],
//                     None,
//                     f::n::x(Expression::BinaryOperation(
//                         BinaryOperator::And,
//                         Box::new(f::n::x(Expression::Identifier(str!("bar")))),
//                         Box::new(f::n::x(Expression::Identifier(str!("fizz")))),
//                     )),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::func_(
//                                 "foo",
//                                 vec![
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("bar"),
//                                             value_type: None,
//                                             default_value: None,
//                                         },
//                                         Ok(Type::Boolean)
//                                     ),
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("fizz"),
//                                             value_type: None,
//                                             default_value: None,
//                                         },
//                                         Ok(Type::Boolean)
//                                     ),
//                                 ],
//                                 None,
//                                 f::n::xc(
//                                     Expression::BinaryOperation(
//                                         BinaryOperator::And,
//                                         Box::new(f::n::xc(
//                                             Expression::Identifier(str!("bar")),
//                                             Ok(Type::Boolean)
//                                         )),
//                                         Box::new(f::n::xc(
//                                             Expression::Identifier(str!("fizz")),
//                                             Ok(Type::Boolean)
//                                         )),
//                                     ),
//                                     Ok(Type::Boolean)
//                                 )
//                             ),
//                             Ok(Type::Function(vec![0, 1], 2))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 3)]))
//                 ))
//             );
//         }
//     }

//     mod view {
//         use super::*;
//         use lang::ast::{BinaryOperator, Parameter};

//         #[test]
//         fn no_parameters() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::view(
//                     "foo",
//                     vec![],
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::view(
//                                 "foo",
//                                 vec![],
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::View(vec![]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 1)]))
//                 ))
//             );
//         }

//         #[test]
//         fn parameters_with_defaults() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::view(
//                     "foo",
//                     vec![
//                         f::n::p(Parameter {
//                             name: str!("bar"),
//                             value_type: None,
//                             default_value: Some(f::n::x(Expression::Primitive(
//                                 Primitive::Boolean(true),
//                             ))),
//                         }),
//                         f::n::p(Parameter {
//                             name: str!("fizz"),
//                             value_type: None,
//                             default_value: Some(f::n::x(Expression::Primitive(
//                                 Primitive::Integer(123),
//                             ))),
//                         }),
//                     ],
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::view(
//                                 "foo",
//                                 vec![
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("bar"),
//                                             value_type: None,
//                                             default_value: Some(f::n::xc(
//                                                 Expression::Primitive(Primitive::Boolean(true),),
//                                                 Ok(Type::Boolean)
//                                             )),
//                                         },
//                                         Ok(Type::Boolean)
//                                     ),
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("fizz"),
//                                             value_type: None,
//                                             default_value: Some(f::n::xc(
//                                                 Expression::Primitive(Primitive::Integer(123)),
//                                                 Ok(Type::Integer)
//                                             )),
//                                         },
//                                         Ok(Type::Integer)
//                                     ),
//                                 ],
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::View(vec![1, 3]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 5)]))
//                 ))
//             );
//         }

//         #[test]
//         fn parameters_with_typedefs() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::view(
//                     "foo",
//                     vec![
//                         f::n::p(Parameter {
//                             name: str!("bar"),
//                             value_type: Some(f::n::tx(TypeExpression::Boolean)),
//                             default_value: None,
//                         }),
//                         f::n::p(Parameter {
//                             name: str!("fizz"),
//                             value_type: Some(f::n::tx(TypeExpression::Integer)),
//                             default_value: None,
//                         }),
//                     ],
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::view(
//                                 "foo",
//                                 vec![
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("bar"),
//                                             value_type: Some(f::n::txc(
//                                                 TypeExpression::Boolean,
//                                                 Ok(Type::Boolean)
//                                             )),
//                                             default_value: None,
//                                         },
//                                         Ok(Type::Boolean)
//                                     ),
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("fizz"),
//                                             value_type: Some(f::n::txc(
//                                                 TypeExpression::Integer,
//                                                 Ok(Type::Integer)
//                                             )),
//                                             default_value: None,
//                                         },
//                                         Ok(Type::Integer)
//                                     ),
//                                 ],
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::View(vec![1, 3]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 5)]))
//                 ))
//             );
//         }

//         #[test]
//         fn parameter_with_default_and_typedef() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::view(
//                     "foo",
//                     vec![f::n::p(Parameter {
//                         name: str!("bar"),
//                         value_type: Some(f::n::tx(TypeExpression::Boolean)),
//                         default_value: Some(f::n::x(Expression::Primitive(Primitive::Integer(
//                             123,
//                         )))),
//                     })],
//                     f::n::x(Expression::Primitive(Primitive::Nil)),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::view(
//                                 "foo",
//                                 vec![f::n::pc(
//                                     Parameter {
//                                         name: str!("bar"),
//                                         value_type: Some(f::n::txc(
//                                             TypeExpression::Boolean,
//                                             Ok(Type::Boolean)
//                                         )),
//                                         default_value: Some(f::n::xc(
//                                             Expression::Primitive(Primitive::Integer(123)),
//                                             Ok(Type::Integer)
//                                         )),
//                                     },
//                                     Ok(Type::Boolean)
//                                 )],
//                                 f::n::xc(Expression::Primitive(Primitive::Nil), Ok(Type::Nil))
//                             ),
//                             Ok(Type::View(vec![2]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 4)]))
//                 ))
//             );
//         }

//         #[test]
//         fn infer_return_type_from_parameter() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::view(
//                     "foo",
//                     vec![f::n::p(Parameter {
//                         name: str!("bar"),
//                         value_type: Some(f::n::tx(TypeExpression::Boolean)),
//                         default_value: None,
//                     })],
//                     f::n::x(Expression::Identifier(str!("bar"))),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::view(
//                                 "foo",
//                                 vec![f::n::pc(
//                                     Parameter {
//                                         name: str!("bar"),
//                                         value_type: Some(f::n::txc(
//                                             TypeExpression::Boolean,
//                                             Ok(Type::Boolean)
//                                         )),
//                                         default_value: None,
//                                     },
//                                     Ok(Type::Boolean)
//                                 )],
//                                 f::n::xc(Expression::Identifier(str!("bar")), Ok(Type::Boolean))
//                             ),
//                             Ok(Type::View(vec![1]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 3)]))
//                 ))
//             );
//         }

//         #[test]
//         #[ignore = "todo"]
//         fn infer_parameters() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::view(
//                     "foo",
//                     vec![
//                         f::n::p(Parameter {
//                             name: str!("bar"),
//                             value_type: None,
//                             default_value: None,
//                         }),
//                         f::n::p(Parameter {
//                             name: str!("fizz"),
//                             value_type: None,
//                             default_value: None,
//                         }),
//                     ],
//                     f::n::x(Expression::BinaryOperation(
//                         BinaryOperator::And,
//                         Box::new(f::n::x(Expression::Identifier(str!("bar")))),
//                         Box::new(f::n::x(Expression::Identifier(str!("fizz")))),
//                     )),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::view(
//                                 "foo",
//                                 vec![
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("bar"),
//                                             value_type: None,
//                                             default_value: None,
//                                         },
//                                         Ok(Type::Boolean)
//                                     ),
//                                     f::n::pc(
//                                         Parameter {
//                                             name: str!("fizz"),
//                                             value_type: None,
//                                             default_value: None,
//                                         },
//                                         Ok(Type::Boolean)
//                                     ),
//                                 ],
//                                 f::n::xc(
//                                     Expression::BinaryOperation(
//                                         BinaryOperator::And,
//                                         Box::new(f::n::xc(
//                                             Expression::Identifier(str!("bar")),
//                                             Ok(Type::Boolean)
//                                         )),
//                                         Box::new(f::n::xc(
//                                             Expression::Identifier(str!("fizz")),
//                                             Ok(Type::Boolean)
//                                         )),
//                                     ),
//                                     Ok(Type::Boolean)
//                                 )
//                             ),
//                             Ok(Type::View(vec![0, 1]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Value, 3)]))
//                 ))
//             );
//         }
//     }

//     mod module {
//         use super::*;

//         #[test]
//         fn empty() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::module(
//                     "foo",
//                     f::n::mr(Module::new(vec![], vec![])),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::module(
//                                 "foo",
//                                 ModuleNode(Module::new(vec![], vec![]), Ok(Type::Module(vec![]))),
//                             ),
//                             Ok(Type::Module(vec![]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Mixed, 1)]))
//                 ))
//             );
//         }

//         #[test]
//         fn with_declarations() {
//             let ast = Program(f::n::mr(Module::new(
//                 vec![],
//                 vec![f::n::d(f::a::module(
//                     "foo",
//                     f::n::mr(Module::new(
//                         vec![],
//                         vec![
//                             f::n::d(f::a::type_("bar", f::n::tx(TypeExpression::Boolean))),
//                             f::n::d(f::a::const_(
//                                 "fizz",
//                                 None,
//                                 f::n::x(Expression::Primitive(Primitive::Integer(123))),
//                             )),
//                         ],
//                     )),
//                 ))],
//             )));
//             let modules = HashMap::from_iter(vec![]);

//             assert_eq!(
//                 super::super::analyze(&mock_reference("foo"), &ast, &modules),
//                 Program(ModuleNode(
//                     Module::new(
//                         vec![],
//                         vec![f::n::dc(
//                             f::a::module(
//                                 "foo",
//                                 ModuleNode(
//                                     Module::new(
//                                         vec![],
//                                         vec![
//                                             f::n::dc(
//                                                 f::a::type_(
//                                                     "bar",
//                                                     f::n::txc(
//                                                         TypeExpression::Boolean,
//                                                         Ok(Type::Boolean)
//                                                     )
//                                                 ),
//                                                 Ok(Type::Boolean)
//                                             ),
//                                             f::n::dc(
//                                                 f::a::const_(
//                                                     "fizz",
//                                                     None,
//                                                     f::n::xc(
//                                                         Expression::Primitive(Primitive::Integer(
//                                                             123
//                                                         )),
//                                                         Ok(Type::Integer)
//                                                     ),
//                                                 ),
//                                                 Ok(Type::Integer)
//                                             ),
//                                         ]
//                                     ),
//                                     Ok(Type::Module(vec![
//                                         (str!("bar"), RefKind::Type, 1),
//                                         (str!("fizz"), RefKind::Value, 3),
//                                     ]))
//                                 ),
//                             ),
//                             Ok(Type::Module(vec![
//                                 (str!("bar"), RefKind::Type, 1),
//                                 (str!("fizz"), RefKind::Value, 3),
//                             ]))
//                         )]
//                     ),
//                     Ok(Type::Module(vec![(str!("foo"), RefKind::Mixed, 5)]))
//                 ))
//             );
//         }
//     }
// }
