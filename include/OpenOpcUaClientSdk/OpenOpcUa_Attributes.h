/*****************************************************************************
Author
©. Michel Condemine, 4CE Industry (2010-2012)

Contributors


This software is a computer program whose purpose is to
implement behavior describe in the OPC UA specification.
see wwww.opcfoundation.org for more details about OPC.
This software is governed by the CeCILL-C license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL-C
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-C license and that you accept its terms.

*****************************************************************************/

#ifndef _OpcUa_Attributes_H_
#define _OpcUa_Attributes_H_ 1

OPCUA_BEGIN_EXTERN_C

		/*============================================================================
		 * The canonical identifier for the node.
		 *===========================================================================*/
#define OpcUa_Attributes_NodeId 1

		 /*============================================================================
		  * The class of the node.
		  *===========================================================================*/
#define OpcUa_Attributes_NodeClass 2

		  /*============================================================================
		   * A non-localized, human readable name for the node.
		   *===========================================================================*/
#define OpcUa_Attributes_BrowseName 3

		   /*============================================================================
			* A localized, human readable name for the node.
			*===========================================================================*/
#define OpcUa_Attributes_DisplayName 4

			/*============================================================================
			 * A localized description for the node.
			 *===========================================================================*/
#define OpcUa_Attributes_Description 5

			 /*============================================================================
			  * Indicates which attributes are writeable.
			  *===========================================================================*/
#define OpcUa_Attributes_WriteMask 6

			  /*============================================================================
			   * Indicates which attributes are writeable by the current user.
			   *===========================================================================*/
#define OpcUa_Attributes_UserWriteMask 7

			   /*============================================================================
				* Indicates that a type node may not be instantiated.
				*===========================================================================*/
#define OpcUa_Attributes_IsAbstract 8

				/*============================================================================
				 * Indicates that forward and inverse references have the same meaning.
				 *===========================================================================*/
#define OpcUa_Attributes_Symmetric 9

				 /*============================================================================
				  * The browse name for an inverse reference.
				  *===========================================================================*/
#define OpcUa_Attributes_InverseName 10

				  /*============================================================================
				   * Indicates that following forward references within a view will not cause a loop.
				   *===========================================================================*/
#define OpcUa_Attributes_ContainsNoLoops 11

				   /*============================================================================
					* Indicates that the node can be used to subscribe to events.
					*===========================================================================*/
#define OpcUa_Attributes_EventNotifier 12

					/*============================================================================
					 * The value of a variable.
					 *===========================================================================*/
#define OpcUa_Attributes_Value 13

					 /*============================================================================
					  * The node id of the data type for the variable value.
					  *===========================================================================*/
#define OpcUa_Attributes_DataType 14

					  /*============================================================================
					   * The number of dimensions in the value.
					   *===========================================================================*/
#define OpcUa_Attributes_ValueRank 15

					   /*============================================================================
						* The length for each dimension of an array value.
						*===========================================================================*/
#define OpcUa_Attributes_ArrayDimensions 16

						/*============================================================================
						 * How a variable may be accessed.
						 *===========================================================================*/
#define OpcUa_Attributes_AccessLevel 17

						 /*============================================================================
						  * How a variable may be accessed after taking the user's access rights into account.
						  *===========================================================================*/
#define OpcUa_Attributes_UserAccessLevel 18

						  /*============================================================================
						   * Specifies (in milliseconds) how fast the server can reasonably sample the value for changes.
						   *===========================================================================*/
#define OpcUa_Attributes_MinimumSamplingInterval 19

						   /*============================================================================
							* Specifies whether the server is actively collecting historical data for the variable.
							*===========================================================================*/
#define OpcUa_Attributes_Historizing 20

							/*============================================================================
							 * Whether the method can be called.
							 *===========================================================================*/
#define OpcUa_Attributes_Executable 21

							 /*============================================================================
							  * Whether the method can be called by the current user.
							  *===========================================================================*/
#define OpcUa_Attributes_UserExecutable 22

OPCUA_END_EXTERN_C

#endif
