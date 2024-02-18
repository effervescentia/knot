mod binary_operation;
mod dot_access;
// mod function_call;
mod identifier;
// mod module;
mod inherit;
mod partial;

use crate::{data::AnalyzeContext, strong, weak};
// use kore::invariant;
// use lang::{types, Fragment, NodeId, ScopeId};

// pub struct Visitor<'a> {
//     next_id: usize,
//     strong: &'a StrongResult<'a>,
// }

// impl<'a> Visitor<'a> {
//     fn next_id(&mut self) -> usize {
//         let id = self.next_id;
//         self.next_id += 1;
//         id
//     }

//     pub fn next_type(&mut self) -> TypeRef<'a> {
//         let id = self.next_id();
//         self.strong
//             .refs
//             .get(&NodeId(id))
//             .unwrap_or_else(|| invariant!("unable to find type reference"))
//     }
// }

// impl<'a> walk::Visit for Visitor<'a> {
//     type Binding = ast::typed::Binding;

//     type Expression = ast::typed::Expression<'a>;

//     type Statement = ast::typed::Statement<'a>;

//     type Component = ast::typed::Component<'a>;

//     type TypeExpression = ast::typed::TypeExpression<'a>;

//     type Parameter = ast::typed::Parameter<'a>;

//     type Declaration = ast::typed::Declaration<'a>;

//     type Import = ast::typed::Import<'a>;

//     type Module = ast::typed::Module<'a>;

//     fn binding(self, x: ast::Binding, r: lang::Range) -> (Self::Binding, Self) {
//         todo!()
//     }

//     fn expression(
//         self,
//         x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
//         r: lang::Range,
//     ) -> (Self::Expression, Self) {
//         todo!()
//     }

//     fn statement(
//         self,
//         x: ast::Statement<Self::Expression>,
//         r: lang::Range,
//     ) -> (Self::Statement, Self) {
//         todo!()
//     }

//     fn component(
//         self,
//         x: ast::Component<Self::Component, Self::Expression>,
//         r: lang::Range,
//     ) -> (Self::Component, Self) {
//         todo!()
//     }

//     fn type_expression(
//         self,
//         x: ast::TypeExpression<Self::TypeExpression>,
//         r: lang::Range,
//     ) -> (Self::TypeExpression, Self) {
//         todo!()
//     }

//     fn parameter(
//         self,
//         x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
//         r: lang::Range,
//     ) -> (Self::Parameter, Self) {
//         todo!()
//     }

//     fn declaration(
//         self,
//         x: ast::Declaration<
//             Self::Binding,
//             Self::Expression,
//             Self::TypeExpression,
//             Self::Parameter,
//             Self::Module,
//         >,
//         r: lang::Range,
//     ) -> (Self::Declaration, Self) {
//         todo!()
//     }

//     fn import(self, x: ast::Import, r: lang::Range) -> (Self::Import, Self) {
//         todo!()
//     }

//     fn module(
//         self,
//         x: ast::Module<Self::Import, Self::Declaration>,
//         r: lang::Range,
//     ) -> (Self::Module, Self) {
//         todo!()
//     }
// }

// pub trait ToStrong<R> {
//     fn to_strong(&self, ctx: &StrongResult) -> R;
// }

pub fn infer_types<'a>(ctx: &AnalyzeContext, weak: weak::Result) -> strong::Result<'a> {
    let mut partial = partial::Result::new(weak.to_descriptors());
    let mut result = strong::Result::new(weak.module);

    while !partial.is_done() {
        let (next_partial, next_result) = partial::infer_types(ctx, partial, result);

        partial = next_partial;
        result = next_result;
    }

    result
}
