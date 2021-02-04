<h1>Rapport Projet Editeur de Text</h1>
<h2>Affichage</h2>
L'éditeur  est sensé fonctionner à partir de n'importe quel dimension.

Problème: des problèmes sont parfois intervenus lors de mes tests quand le fichier passé en argument a des lignes de tailles plus grandes que le terminal...


<h2> Structure de donnée </h2>

J'ai utilisé , une structure où je stock tous les caractères dans un tableau de liste chaînée.
J'ai fait ce choix pour simplifier les insertions, suppressions et ajouts.

J'ai une structure  cursor_xy_t pour stocker la position du curseur et de la souris.
J'ai unes structure cursor_max_t  qui me permet d'établir la colonnne max atteignable sur chaque lignes et le nombre de ligne max .Cela me permet de ne pas avoir d'effet de bord
sur mon tableau.




<h2>Insertion</h2>
J'ai rencontré plusieurs problème pour supprimer un caractère en fin de ligne
et faire un backspace en début de  ligne. La raison est que je ne supprimais 
pas bien les \n \r lorsque je liais de ligne. Cela fonctionne normalement maintenant.

Les flèches fonctionnent bien et on ne peut pas aller en dehors du texte.

On notera que je réaffiche beaucoup de fois le texte, cela cause un gros problème
que je n'ai pas réglé . Lorsque que l'on reste appuyés sur une touche , le texte se réaffiche trop de fois en peu de temps donc il y a un effet de clignotement.

Problème rencontré que je n'ai pas eu le temps de reglé:
Certains caractère spéciaux crééent  des décalages par exemple le "è" et le "é", décale mon curseur de une colonne de plus que tous les autres caractères.

En appuyant sur echap on passe en mode edition.

<h2>Edition</h2>

Les flèches fonctionnent,entrer nous fait aller à la ligne suivante et backspace est comme la flèche de gauche.


Ici j'ai implémenté les commandes de déplacement suivantes<br>
    <ul>
    <li> i: retour au mode insertion</li>
    <li>w: pour aller au mot suivant </li>
    <li>b: pour le mot précèdent </li>
    <li>$: fin de phrase</li>
    <li>O: début de phrase</li>
    <li>G: dernière ligne du text</li>
    <li>g: première ligne du text</li>
    <li>L: bas d'écran</li>
    <li>M: milieu d'écran</li>
    
    </ul>

J'ai implémenté uniquement le x pour supprimer un caractère. La touche suppr fonctionne aussi.


J'ai pas eu le temps de faire les action de copie et suppression :
    J'ai uniquement implémenté la copie jusqu'à la fin de la ligne, avec la touche c. Elle fonctionne je l'ai vérifié , et c'est vérifiable rapidement avec un printf.
    

<h4>Commande </h4>
On appuie sur : ,puis :
 <ul>
    <li>w: pour enregister</li>
    <li>q :pour quitter </li>
    <li>qw:pour quitter et enregister</li>
    </ul>





<h2> Souris </h2>

Les deux modes déplacement fonctionne bien  en même temps.
J'ai bien créé un deuxième curseur , il fonctionne parfaitement après mes test.
J'ai créé un système de vitesse de souris , la commande :vn avec n entre 1 et 9  permet de varier la vitesse.

Le clic positionne le premier curseur à l'endroit de la souris et si c'est en dehors du text, on le positionne au dernier caractère de la ligne.




<h2> Amélioration </h2>

J'ai implémenté la recherche de mot et l'algorithme de knut morris path. On peut aller vers l'avant sur tout le texte ,et revenir en arrière jusqu'à la première ligne afficher en haut du terminal.
Les mots trouvés sont  surligner en jaune .

En mode edition il faut appuyer sur / puis écrire le mot.