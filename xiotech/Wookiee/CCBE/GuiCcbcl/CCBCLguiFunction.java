/* $Header$*/
/*===========================================================================
** FILE NAME:       CCBCLguiFunction.java
** MODULE TITLE:
** AUTHOR:          Bryan Holty
** DATE:            2/1/2001
**
** DESCRIPTION:     Encapsulates a function from the ccbcl.pl
**
** Copyright (c) 2001  XIOtech a Seagate Company.  All rights reserved.
**==========================================================================*/
import java.util.Vector;

/*===========================================================================
** CCBCLguiFunction
**===========================================================================*/
public class CCBCLguiFunction implements Comparable
{
    /*===========================================================================
    ** Constructor 
    **
    ** Inputs:  name:       - name of function
    **
    ** Returns: none 
    **===========================================================================*/
    public CCBCLguiFunction(String name)
    {
        super();
        funcName = new String(name);
        parameters = new Vector();
        help = new String("");
    }
    
    /*===========================================================================
    ** Constructor 
    **
    ** Inputs:  name:       - name of function
    **          hlp:        - help text    
    **
    ** Returns: none 
    **===========================================================================*/
    public CCBCLguiFunction(String name, String hlp)
    {
        super();
        funcName = new String(name);
        parameters = new Vector();
        help = new String(hlp);
    }
    
    /*===========================================================================
    ** addParameter     adds a parameter name to the parameters vector
    **
    ** Inputs:          parm:   - Name of parameter
    **
    ** Returns: none 
    **===========================================================================*/
    public void addParameter(String parm)
    {
        parameters.add(parm);
    }
    
    /*===========================================================================
    ** removeParameter  removes a parameter name from the parameters vector
    **
    ** Inputs:          parm:   - Name of parameter
    **
    ** Returns: none
    **
    ** Throws:          Exception 
    **===========================================================================*/
    public void removeParameter(String parm)throws Exception
    {
        if(!(parameters.remove(parm)))
        {
            throw new Exception("Parameter does not exist");
        }
    }
    
    /*===========================================================================
    ** getParameters    returns a list of the parameters
    **
    ** Inputs:  none
    **
    ** Returns:         String[] - an array of Strings (parameter names)
    **===========================================================================*/
    public String[] getParameters()
    {
        int i = parameters.size();
        int j = (parameters.size() - 1);
        String[] strs = new String[i];
        
        while (i > 0)
        {
            --i;
            strs[j-i] = new String((String)parameters.elementAt(i));
        }
        
        return strs;
    }
    
    /*===========================================================================
    ** getHelp          returns the help text
    **
    ** Inputs:  none
    **
    ** Returns:         String - help text String 
    **===========================================================================*/
    public String getHelp()
    {
        return new String(help);
    }
    
    /*===========================================================================
    ** setHelp          sets the help text
    **
    ** Inputs:          str:   - help text
    **
    ** Returns: none 
    **===========================================================================*/
    public void setHelp(String str)
    {
        help = new String(str);
    }   
    
    /*===========================================================================
    ** getFuncName      returns the name of this function
    **
    ** Inputs:  none
    **
    ** Returns:         String - name of function
    **===========================================================================*/
    public String getFuncName()
    {
        return new String (funcName);
    }
    
    /*===========================================================================
    ** setFuncName      sets the name of this function
    **
    ** Inputs:          str:   - name of function
    **
    ** Returns: none 
    **===========================================================================*/
    public void setFuncName(String str)
    {
        funcName = new String(str);
    }
    
    /*===========================================================================
    ** compareTo        compares this function to another function
    **
    ** Inputs:          tmp:   - Function compare to
    **
    ** Returns:         int:  < 0: less than
    **                        > 0: greater than
    **                        = 0: equal to
    **===========================================================================*/
    public int compareTo(Object tmp)
    {
        return funcName.compareTo(((CCBCLguiFunction)tmp).getFuncName());
    }
    
    /*===========================================================================
    ** toString         Puts this function into a String format
    **
    ** Inputs:  none
    **
    ** Returns:         String - this to String 
    **===========================================================================*/
    public String toString()
    {
        String tmp = new String("");
        String tmparr[] = this.getParameters();
        int i;
        
        tmp += "Function   : " + funcName + "\n";
        tmp += "Help:\n" + help;
        
        for (i = 0; i < tmparr.length; ++i)
        {
            tmp += "Parameter " + (i+1) + ": " + tmparr[i] + "\n";
        }
        
        tmp += "\n";
        
        return new String(tmp);
    }
    
            
    /*===========================================================================
    ** Private Variables
    **===========================================================================*/
    
    private Vector parameters;  /* Vector of parameter names */
    private String help;        /* help text                 */
    private String funcName;    /* name of this function     */
}


/****************************************************************************
** $Log$
** Revision 1.1  2005/05/04 18:53:54  RysavyR
** Initial revision
**
** Revision 1.1  2002/02/12 21:01:10  HoltyB
** initial integration of GUICCBCL
**
****************************************************************************/