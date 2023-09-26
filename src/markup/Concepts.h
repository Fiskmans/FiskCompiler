
#ifndef MARKUP_CONCEPTS_H
#define MARKUP_CONCEPTS_H

namespace markup
{
	template<class LeftHand, class RightHand>
	concept AssingableBy = requires(LeftHand lhs, RightHand rhs)
	{
		lhs = rhs;
	};
}

#endif